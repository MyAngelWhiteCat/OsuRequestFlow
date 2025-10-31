#include "ca_sertificates_loader.h"

namespace ssl_domain_utilities{

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

}