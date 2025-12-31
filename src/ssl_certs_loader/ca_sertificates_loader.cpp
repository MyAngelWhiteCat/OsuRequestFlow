#include "ca_sertificates_loader.h"

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/impl/context.ipp>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include <openssl/x509_vfy.h>


#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#include <openssl/x509.h>
#endif

namespace ssl_domain_utilities{

// AI on
#ifdef _WIN32

    void load_windows_ca_certificates(ssl::context& ctx) {
        HCERTSTORE hStore = CertOpenSystemStoreA(NULL, "ROOT");
        if (!hStore) {
            return;
        }

        PCCERT_CONTEXT pContext = nullptr;
        while ((pContext = CertEnumCertificatesInStore(hStore, pContext))) {
            const unsigned char* cert_data = pContext->pbCertEncoded;
            X509* x509 = d2i_X509(nullptr, &cert_data, pContext->cbCertEncoded);
            if (x509) {
                X509_STORE* store = SSL_CTX_get_cert_store(ctx.native_handle());
                if (store) {
                    X509_STORE_add_cert(store, x509);
                }
                X509_free(x509);
            }
        }

        CertCloseStore(hStore, 0);
    }

#else

void load_windows_ca_certificates(ssl::context& ctx) {
    (void)ctx;
}
#endif
// AI off

}