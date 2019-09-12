// Copyright 2005-2019 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifdef MUMBLE
	#include "mumble_pch.hpp"
#else
	#include "murmur_pch.h"
#endif

#include "CryptographicHash.h"

class CryptographicHashPrivate {
	public:
		CryptographicHashPrivate(const EVP_MD *type);
		~CryptographicHashPrivate();
		void addData(const QByteArray &buf);
		QByteArray result();

	private:
		EVP_MD_CTX *m_mdctx;

		/// The result of the cryptographic hash.
		/// This field is filled out by the result()
		/// method.
		/// Once this field is set, the m_mdctx is finalized
		/// and can't be used anymore, so result() and addData()
		/// must guard against that possibility.
		QByteArray m_result;

		CryptographicHashPrivate();

		/// If m_mdctx is set, cleanupMdctx will destroy
		/// the EVP_MD_CTX object and set m_mdctx to NULL.
		void cleanupMdctx();
};

CryptographicHashPrivate::CryptographicHashPrivate()
	: m_mdctx(NULL) {
}

void CryptographicHashPrivate::cleanupMdctx() {
	if (m_mdctx) {
		EVP_MD_CTX_destroy(m_mdctx);
		m_mdctx = NULL;
	}
}

CryptographicHashPrivate::CryptographicHashPrivate(const EVP_MD *type)
	: m_mdctx(NULL) {

	m_mdctx = EVP_MD_CTX_create();
	if (m_mdctx == NULL) {
		return;
	}

	int err = EVP_DigestInit_ex(m_mdctx, type, NULL);
	if (err != 1) {
		cleanupMdctx();
		return;
	}
}

CryptographicHashPrivate::~CryptographicHashPrivate() {
	cleanupMdctx();
}

void CryptographicHashPrivate::addData(const QByteArray &buf) {
	if (m_mdctx == NULL) {
		return;
	}

	// If we have a result already, that means our m_mdctx
	// object has been finalized, and can't be used anymore.
	// In that case, transition into an error state by cleaning
	// up the mdctx -- that way, subsequent calls to result()
	// will return an empty QByteArray.
	if (!m_result.isNull()) {
		cleanupMdctx();
		return;
	}

	int err = EVP_DigestUpdate(m_mdctx, buf.constData(), buf.size());
	if (err != 1) {
		cleanupMdctx();
	}
}

QByteArray CryptographicHashPrivate::result() {
	if (m_mdctx == NULL) {
		return QByteArray();
	}

	// If we have a result already, that means our m_mdctx
	// object has been finalized, and can't be used anymore.
	// In that case, just return the already computed result.
	if (!m_result.isNull()) {
		return m_result;
	}

	QByteArray digest(EVP_MD_CTX_size(m_mdctx), '\0');
	int err = EVP_DigestFinal_ex(m_mdctx, reinterpret_cast<unsigned char *>(digest.data()), NULL);
	if (err != 1) {
		cleanupMdctx();
		return QByteArray();
	}

	m_result = digest;

	return m_result;
}

QByteArray CryptographicHash::hash(const QByteArray &buf, Algorithm algo) {
	CryptographicHash h(algo);
	h.addData(buf);
	return h.result();
}

QString CryptographicHash::humanReadableAlgorithmName(CryptographicHash::Algorithm algo) {
	switch (algo) {
		case CryptographicHash::Sha1:
			return QLatin1String("SHA-1");
		case CryptographicHash::Sha256:
			return QLatin1String("SHA-256");
	}
	return QString();
}

QString CryptographicHash::shortAlgorithmName(CryptographicHash::Algorithm algo) {
	switch (algo) {
		case CryptographicHash::Sha1:
			return QLatin1String("sha1");
		case CryptographicHash::Sha256:
			return QLatin1String("sha256");
	}
	return QString();
}

CryptographicHash::CryptographicHash()
	: d(NULL) {
}

CryptographicHash::CryptographicHash(CryptographicHash::Algorithm algo)
	: d(NULL) {

	switch (algo) {
		case CryptographicHash::Sha1:
			d = new CryptographicHashPrivate(EVP_sha1());
			break;
		case CryptographicHash::Sha256:
			d = new CryptographicHashPrivate(EVP_sha256());
			break;
	}
}

void CryptographicHash::addData(const QByteArray &buf) {
	if (d) {
		d->addData(buf);
	}
}

QByteArray CryptographicHash::result() const {
	if (d) {
		return d->result();
	}
	return QByteArray();
}
