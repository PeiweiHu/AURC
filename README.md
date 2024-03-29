# AURC - API Usage References consistency Checker

This is the repository of the study *AURC: Detecting Errors in Program Code and Documentation* (accepted by USENIX Security 2023).

## Usage

### 1. Compile

After compiling by `make`, the binary `main` appears.

### 2. Generate the Information from Three AURs

**Callee:**

```bash
./main --cbp `python script/extract_bc.py -p bitcode/openssl`
```

**Caller:**

```bash
./main --cops `python script/extract_bc.py -p bitcode/openssl`
```

**Document:**

You can train your own model based on the codebase you wanna test. We use
bare Bert. The result is good enough. We provide the doc info of OpenSSL in
`result/doc_openssl.json`.

### 3. Compare:

```bash
python cmp.py --cbp=./result/return_values.txt --cops=./result/cops.txt --doc=./result/doc_openssl.json --mapping=./result/macro_enum.txt --nonexist_pos --nonexist_neg --confuse_pos_and_neg_ssl -s 
```

## Trophy

AURC pays attention to the practical values of refining real-world software. With the help of AURC, hundreds of defective return checks and document defects are detected and fixed, as shown in the following two tables. The first table shows code patches, while the second presents the document patches.

| Id | Codebase | Caller | Callee | Commit |
| -- | -------- | ------ | ------ | ------ |
|0|OpenSSL|EVP_CIPHER_CTX_get_updated_iv|evp_do_ciph_ctx_getparams|dd1f28427b375931fda45180619c8f5971cd6bca|
|1|OpenSSL|EVP_CIPHER_CTX_get_original_iv|evp_do_ciph_ctx_getparams|dd1f28427b375931fda45180619c8f5971cd6bca|
|2|OpenSSL|RSA_sign|rsa_sign|dd1f28427b375931fda45180619c8f5971cd6bca|
|3|OpenSSL|X509v3_get_ext_count|sk_X509_EXTENSION_num|dd1f28427b375931fda45180619c8f5971cd6bca|
|4|OpenSSL|X509_NAME_entry_count|sk_X509_NAME_ENTRY_num|dd1f28427b375931fda45180619c8f5971cd6bca|
|5|OpenSSL|www_body|BIO_set_write_buffer_size|25d02f333b9a5531fa88db294f69a8347f275858|
|6|OpenSSL|rev_body|BIO_set_write_buffer_size|25d02f333b9a5531fa88db294f69a8347f275858|
|7|OpenSSL|buffer_ctrl|BIO_set_read_buffer_size|25d02f333b9a5531fa88db294f69a8347f275858|
|8|OpenSSL|buffer_ctrl|BIO_set_write_buffer_size|25d02f333b9a5531fa88db294f69a8347f275858|
|9|OpenSSL|linebuffer_ctrl|BIO_set_write_buffer_size|25d02f333b9a5531fa88db294f69a8347f275858|
|10|OpenSSL|ssl_init_wbio_buffer|BIO_set_read_buffer_size|25d02f333b9a5531fa88db294f69a8347f275858|
|11|OpenSSL|OSSL_CRMF_ENCRYPTEDVALUE_get1_encCert|EVP_PKEY_decrypt_init|b794476df71441a6d30740ab9fadcc0f6d18d3d6|
|12|OpenSSL|test_EVP_SM2|EVP_PKEY_decrypt_init|b794476df71441a6d30740ab9fadcc0f6d18d3d6|
|13|OpenSSL|rsautl_main|EVP_PKEY_verify_recover_init|4c3fadfe57b94f71fa83786726046b8833997c7c|
|14|OpenSSL|rsautl_main|EVP_PKEY_CTX_set_rsa_padding|4c3fadfe57b94f71fa83786726046b8833997c7c|
|15|OpenSSL|rsautl_main|EVP_PKEY_verify_recover|4c3fadfe57b94f71fa83786726046b8833997c7c|
|16|OpenSSL|rsautl_main|EVP_PKEY_sign_init|4c3fadfe57b94f71fa83786726046b8833997c7c|
|17|OpenSSL|rsautl_main|EVP_PKEY_CTX_set_rsa_padding|4c3fadfe57b94f71fa83786726046b8833997c7c|
|18|OpenSSL|rsautl_main|EVP_PKEY_sign|4c3fadfe57b94f71fa83786726046b8833997c7c|
|19|OpenSSL|rsautl_main|EVP_PKEY_encrypt_init|4c3fadfe57b94f71fa83786726046b8833997c7c|
|20|OpenSSL|rsautl_main|EVP_PKEY_CTX_set_rsa_padding|4c3fadfe57b94f71fa83786726046b8833997c7c|
|21|OpenSSL|rsautl_main|EVP_PKEY_encrypt|4c3fadfe57b94f71fa83786726046b8833997c7c|
|22|OpenSSL|rsautl_main|EVP_PKEY_decrypt_init|4c3fadfe57b94f71fa83786726046b8833997c7c|
|23|OpenSSL|rsautl_main|EVP_PKEY_CTX_set_rsa_padding|4c3fadfe57b94f71fa83786726046b8833997c7c|
|24|OpenSSL|rsautl_main|EVP_PKEY_decrypt|4c3fadfe57b94f71fa83786726046b8833997c7c|
|25|OpenSSL|ec_validate|EC_GROUP_check_named_curve|3b6154ccaf3e64bcdfda4859f2b98ef21b08c5b2|
|26|OpenSSL|speed_main|EVP_PKEY_derive_init|9dd009dd513276e602b6592bc337a8563a1a82a1|
|27|OpenSSL|speed_main|EVP_PKEY_derive_set_peer|9dd009dd513276e602b6592bc337a8563a1a82a1|
|28|OpenSSL|speed_main|EVP_PKEY_derive|9dd009dd513276e602b6592bc337a8563a1a82a1|
|29|OpenSSL|speed_main|EVP_PKEY_derive|9dd009dd513276e602b6592bc337a8563a1a82a1|
|30|OpenSSL|speed_main|EVP_PKEY_derive|9dd009dd513276e602b6592bc337a8563a1a82a1|
|31|OpenSSL|speed_main|EVP_PKEY_derive_init|9dd009dd513276e602b6592bc337a8563a1a82a1|
|32|OpenSSL|speed_main|EVP_PKEY_derive_set_peer|9dd009dd513276e602b6592bc337a8563a1a82a1|
|33|OpenSSL|speed_main|EVP_PKEY_derive|9dd009dd513276e602b6592bc337a8563a1a82a1|
|34|OpenSSL|speed_main|EVP_PKEY_derive|9dd009dd513276e602b6592bc337a8563a1a82a1|
|35|OpenSSL|setup_verify|X509_LOOKUP_load_file_ex|e22ea36fa8296b402348da8f5ab5e258be8402cf|
|36|OpenSSL|setup_verify|X509_LOOKUP_add_dir|e22ea36fa8296b402348da8f5ab5e258be8402cf|
|37|OpenSSL|create_cert_store|X509_LOOKUP_add_dir|e22ea36fa8296b402348da8f5ab5e258be8402cf|
|38|OpenSSL|create_cert_store|X509_LOOKUP_load_file_ex|e22ea36fa8296b402348da8f5ab5e258be8402cf|
|39|OpenSSL|create_cert_store|X509_LOOKUP_load_store_ex|e22ea36fa8296b402348da8f5ab5e258be8402cf|
|40|OpenSSL|X509_STORE_load_file_ex|X509_LOOKUP_load_file_ex|e22ea36fa8296b402348da8f5ab5e258be8402cf|
|41|OpenSSL|X509_STORE_load_path|X509_LOOKUP_add_dir|e22ea36fa8296b402348da8f5ab5e258be8402cf|
|42|OpenSSL|make_revoked|X509_REVOKED_add1_ext_i2d|c540a82767954a616934ba6caa6ddc736502c574|
|43|OpenSSL|make_revoked|X509_REVOKED_add1_ext_i2d|c540a82767954a616934ba6caa6ddc736502c574|
|44|OpenSSL|make_revoked|X509_REVOKED_add1_ext_i2d|c540a82767954a616934ba6caa6ddc736502c574|
|45|OpenSSL|cms_copy_content|BIO_get_cipher_status|48b571fe771f283d547ca2a5999ce5dd9a5509d0|
|46|OpenSSL|PKCS7_decrypt|BIO_get_cipher_status|48b571fe771f283d547ca2a5999ce5dd9a5509d0|
|47|OpenSSL|PKCS7_decrypt|BIO_get_cipher_status|48b571fe771f283d547ca2a5999ce5dd9a5509d0|
|48|OpenSSL|dhparam_main|EVP_PKEY_param_check|e85bef981c037a6ebc0ca39f61c11bd79ed89fb3|
|49|OpenSSL|ecparam_main|EVP_PKEY_param_check|e85bef981c037a6ebc0ca39f61c11bd79ed89fb3|
|50|OpenSSL|file_get_pass|UI_add_input_string|5755c11fd6e50028946e6e17c835afcd56995699|
|51|OpenSSL|test_old|UI_add_input_string|5755c11fd6e50028946e6e17c835afcd56995699|
|52|OpenSSL|test_fromdata_rsa|EVP_PKEY_private_check|babc818c3f669214fa192229003953e3dead1926|
|53|OpenSSL|test_fromdata_dh_named_group|EVP_PKEY_private_check|babc818c3f669214fa192229003953e3dead1926|
|54|OpenSSL|test_fromdata_dh_fips186_4|EVP_PKEY_private_check|babc818c3f669214fa192229003953e3dead1926|
|55|OpenSSL|test_fromdata_ecx|EVP_PKEY_private_check|babc818c3f669214fa192229003953e3dead1926|
|56|OpenSSL|test_fromdata_dsa_fips186_4|EVP_PKEY_private_check|babc818c3f669214fa192229003953e3dead1926|
|57|OpenSSL|test_check_dsa|EVP_PKEY_private_check|babc818c3f669214fa192229003953e3dead1926|
|58|OpenSSL|test_fromdata_rsa|EVP_PKEY_public_check|bba14c6e28e9519b2d40fc5c551893996f2db246|
|59|OpenSSL|test_fromdata_dh_named_group|EVP_PKEY_public_check|bba14c6e28e9519b2d40fc5c551893996f2db246|
|60|OpenSSL|test_fromdata_dh_fips186_4|EVP_PKEY_public_check|bba14c6e28e9519b2d40fc5c551893996f2db246|
|61|OpenSSL|test_fromdata_ecx|EVP_PKEY_public_check|bba14c6e28e9519b2d40fc5c551893996f2db246|
|62|OpenSSL|test_fromdata_dsa_fips186_4|EVP_PKEY_public_check|bba14c6e28e9519b2d40fc5c551893996f2db246|
|63|OpenSSL|test_check_dsa|EVP_PKEY_public_check|bba14c6e28e9519b2d40fc5c551893996f2db246|
|64|OpenSSL|test_fromdata_rsa|EVP_PKEY_pairwise_check|d016758706d0a7a104ff09db94448aeec1b38193|
|65|OpenSSL|test_fromdata_dh_named_group|EVP_PKEY_pairwise_check|d016758706d0a7a104ff09db94448aeec1b38193|
|66|OpenSSL|test_fromdata_dh_fips186_4|EVP_PKEY_pairwise_check|d016758706d0a7a104ff09db94448aeec1b38193|
|67|OpenSSL|test_fromdata_dsa_fips186_4|EVP_PKEY_pairwise_check|d016758706d0a7a104ff09db94448aeec1b38193|
|68|OpenSSL|test_check_dsa|EVP_PKEY_pairwise_check|d016758706d0a7a104ff09db94448aeec1b38193|
|69|OpenSSL|ec_main|EVP_PKEY_check|92d0d7ea9be40909ee79fb8861641a61eead2431|
|70|OpenSSL|rsa_main|EVP_PKEY_check|92d0d7ea9be40909ee79fb8861641a61eead2431|
|71|OpenSSL|test_fromdata_rsa|EVP_PKEY_check|92d0d7ea9be40909ee79fb8861641a61eead2431|
|72|OpenSSL|test_fromdata_dh_named_group|EVP_PKEY_check|92d0d7ea9be40909ee79fb8861641a61eead2431|
|73|OpenSSL|test_fromdata_dh_fips186_4|EVP_PKEY_check|92d0d7ea9be40909ee79fb8861641a61eead2431|
|74|OpenSSL|test_fromdata_dsa_fips186_4|EVP_PKEY_check|92d0d7ea9be40909ee79fb8861641a61eead2431|
|75|OpenSSL|test_fromdata_ecx|EVP_PKEY_check|92d0d7ea9be40909ee79fb8861641a61eead2431|
|76|OpenSSL|test_fromdata_ecx|EVP_PKEY_check|92d0d7ea9be40909ee79fb8861641a61eead2431|
|77|OpenSSL|test_check_dsa|EVP_PKEY_check|92d0d7ea9be40909ee79fb8861641a61eead2431|
|78|OpenSSL|send_record|RAND_bytes|c2f7614fb7b93fe3792068077ff01384f42f39bc|
|79|OpenSSL|test_rand_fork_safety|RAND_bytes|c2f7614fb7b93fe3792068077ff01384f42f39bc|
|80|OpenSSL|test_builtin|RAND_bytes|c2f7614fb7b93fe3792068077ff01384f42f39bc|
|81|OpenSSL|test_mod_exp|RAND_bytes|c2f7614fb7b93fe3792068077ff01384f42f39bc|
|82|OpenSSL|test_mod_exp|RAND_bytes|c2f7614fb7b93fe3792068077ff01384f42f39bc|
|83|OpenSSL|test_mod_exp|RAND_bytes|c2f7614fb7b93fe3792068077ff01384f42f39bc|
|84|OpenSSL|test_WPACKET_init_der|RAND_bytes|c2f7614fb7b93fe3792068077ff01384f42f39bc|
|85|OpenSSL|test_WPACKET_quic_vlint_random|RAND_bytes|c2f7614fb7b93fe3792068077ff01384f42f39bc|
|86|OpenSSL|dhparam_main|EVP_PKEY_CTX_set_dsa_paramgen_bits|2cba2e160d5b028e4a777e8038744a8bc4280629|
|87|OpenSSL|dhparam_main|EVP_PKEY_CTX_set_dh_paramgen_prime_len|2cba2e160d5b028e4a777e8038744a8bc4280629|
|88|OpenSSL|dhparam_main|EVP_PKEY_CTX_set_dh_paramgen_generator|2cba2e160d5b028e4a777e8038744a8bc4280629|
|89|OpenSSL|dsaparam_main|EVP_PKEY_CTX_set_dsa_paramgen_bits|2cba2e160d5b028e4a777e8038744a8bc4280629|
|90|OpenSSL|dh_cms_encrypt|EVP_PKEY_CTX_get_dh_kdf_md|2cba2e160d5b028e4a777e8038744a8bc4280629|
|91|OpenSSL|ecdh_cms_encrypt|EVP_PKEY_CTX_get_ecdh_kdf_md|2cba2e160d5b028e4a777e8038744a8bc4280629|
|92|OpenSSL|rsa_keygen_test|EVP_PKEY_CTX_set_params|2cba2e160d5b028e4a777e8038744a8bc4280629|
|93|OpenSSL|rsa_keygen_test|EVP_PKEY_CTX_set_rsa_keygen_bits|2cba2e160d5b028e4a777e8038744a8bc4280629|
|94|OpenSSL|rsa_keygen_test|EVP_PKEY_CTX_set1_rsa_keygen_pubexp|2cba2e160d5b028e4a777e8038744a8bc4280629|
|95|OpenSSL|test_EC_keygen_with_enc|EVP_PKEY_CTX_set_ec_param_enc|2cba2e160d5b028e4a777e8038744a8bc4280629|
|96|OpenSSL|test_EVP_SM2|EVP_PKEY_CTX_set_ec_paramgen_curve_nid|2cba2e160d5b028e4a777e8038744a8bc4280629|
|97|OpenSSL|test_ecpub|EVP_PKEY_CTX_set_ec_paramgen_curve_nid|2cba2e160d5b028e4a777e8038744a8bc4280629|
|98|OpenSSL|test_EVP_rsa_pss_with_keygen_bits|EVP_PKEY_CTX_set_rsa_pss_keygen_md|2cba2e160d5b028e4a777e8038744a8bc4280629|
|99|OpenSSL|EVP_PKEY_CTX_set0_rsa_oaep_label|evp_pkey_ctx_set_params_strict|00d5193b688019a85d1bd0196f2837a4476394bb|
|100|OpenSSL|rsa_ctx_to_pss|EVP_PKEY_CTX_get_rsa_pss_saltlen|7263a7fc3d0c0c17616c2e5309e0fd52ed654ecc|
|101|OpenSSL|test_EVP_rsa_pss_set_saltlen|EVP_PKEY_CTX_set_rsa_pss_saltlen|7263a7fc3d0c0c17616c2e5309e0fd52ed654ecc|
|102|OpenSSL|test_EVP_rsa_pss_set_saltlen|EVP_PKEY_CTX_get_rsa_pss_saltlen|7263a7fc3d0c0c17616c2e5309e0fd52ed654ecc|
|103|OpenSSL|pkey_type2param|EVP_PKEY_CTX_set_group_name|56876ae952b96b4a83266f6b2ec1393f599015d6|
|104|OpenSSL|ssl_generate_pkey_group|EVP_PKEY_CTX_set_group_name|56876ae952b96b4a83266f6b2ec1393f599015d6|
|105|OpenSSL|ssl_generate_param_group|EVP_PKEY_CTX_set_group_name|56876ae952b96b4a83266f6b2ec1393f599015d6|
|106|OpenSSL|test_EC_keygen_with_enc|EVP_PKEY_CTX_set_group_name|56876ae952b96b4a83266f6b2ec1393f599015d6|
|107|OpenSSL|UI_UTIL_wrap_read_pem_callback|UI_method_set_ex_data|1aef2c10f10e0685298008be596c80e148c71a51|
|108|OpenSSL|multiblock_speed|EVP_CIPHER_CTX_rand_key|f15e3f3aa95df743f0da793da952f87370efb4ff|
|109|OpenSSL|test_cipher_tdes_randkey|EVP_CIPHER_CTX_rand_key|f15e3f3aa95df743f0da793da952f87370efb4ff|
|110|OpenSSL|init_evp_cipher_ctx|EVP_CIPHER_CTX_set_key_length|8d9fec1781751d2106d899c6076eeb3da6930bfe|
|111|OpenSSL|CMAC_Init|EVP_CIPHER_CTX_get0_cipher|8d9fec1781751d2106d899c6076eeb3da6930bfe|
|112|OpenSSL|CMAC_Init|EVP_CIPHER_CTX_set_key_length|8d9fec1781751d2106d899c6076eeb3da6930bfe|
|113|OpenSSL|EVP_OpenInit|EVP_CIPHER_CTX_set_key_length|8d9fec1781751d2106d899c6076eeb3da6930bfe|
|114|OpenSSL|PKCS7_dataDecode|EVP_CIPHER_CTX_set_key_length|8d9fec1781751d2106d899c6076eeb3da6930bfe|
|115|OpenSSL|cipher_init|EVP_CIPHER_CTX_set_key_length|8d9fec1781751d2106d899c6076eeb3da6930bfe|
|116|OpenSSL|badkeylen_test|EVP_CIPHER_CTX_set_key_length|8d9fec1781751d2106d899c6076eeb3da6930bfe|
|117|OpenSSL|EVP_PKEY_get_params|evp_keymgmt_get_params|7e5e91176b770a68bdaf73a5c647f1fc0d7f2900|
|118|OpenSSL|EVP_PKEY_get_params|evp_pkey_get_params_to_ctrl|7e5e91176b770a68bdaf73a5c647f1fc0d7f2900|
|119|OpenSSL|PKCS7_add_signature|PKCS7_SIGNER_INFO_set|639e576023aa2492ca87e1e6503c40d2e8c9a24e|
|120|OpenSSL|PKCS7_add_signature|PKCS7_RECIP_INFO_set|639e576023aa2492ca87e1e6503c40d2e8c9a24e|
|121|OpenSSL||addr_contains|639e576023aa2492ca87e1e6503c40d2e8c9a24e|
|122|OpenSSL||dtls1_process_buffered_records|639e576023aa2492ca87e1e6503c40d2e8c9a24e|
|123|OpenSSL|dtls1_read_bytes|BIO_dgram_sctp_msg_waiting|639e576023aa2492ca87e1e6503c40d2e8c9a24e|
|124|OpenSSL|dtls1_read_bytes|BIO_dgram_sctp_msg_waiting|639e576023aa2492ca87e1e6503c40d2e8c9a24e|
|125|OpenSSL||dtls1_process_record|639e576023aa2492ca87e1e6503c40d2e8c9a24e|
|126|OpenSSL||srp_generate_server_master_secret|639e576023aa2492ca87e1e6503c40d2e8c9a24e|
|127|OpenSSL||srp_generate_client_master_secret|639e576023aa2492ca87e1e6503c40d2e8c9a24e|
|128|OpenSSL|multiblock_speed|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|129|OpenSSL|evp_cipher_init_internal|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|130|OpenSSL|cipher_init|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|131|OpenSSL|cipher_init|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|132|OpenSSL|self_test_cipher|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|133|OpenSSL|gmac_set_ctx_params|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|134|OpenSSL|tls1_change_cipher_state|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|135|OpenSSL|tls1_change_cipher_state|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|136|OpenSSL|tls1_change_cipher_state|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|137|OpenSSL|tls1_change_cipher_state|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|138|OpenSSL|tls1_change_cipher_state|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|139|OpenSSL|derive_secret_key_and_iv|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|140|OpenSSL|derive_secret_key_and_iv|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|141|OpenSSL|aes_ccm_enc_dec|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|142|OpenSSL|aes_ccm_enc_dec|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|143|OpenSSL|aes_ccm_enc_dec|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|144|OpenSSL|aes_gcm_enc_dec|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|145|OpenSSL|aes_gcm_enc_dec|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|146|OpenSSL|aes_gcm_enc_dec|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|147|OpenSSL|evp_init_seq_set_iv|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|148|OpenSSL|test_evp_init_seq|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|149|OpenSSL|test_evp_init_seq|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|150|OpenSSL|test_gcm_reinit|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|151|OpenSSL|test_gcm_reinit|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|152|OpenSSL|test_gcm_reinit|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|153|OpenSSL|test_gcm_reinit|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|154|OpenSSL|cipher_test_enc|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|155|OpenSSL|cipher_test_enc|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|156|OpenSSL|cipher_test_enc|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|157|OpenSSL|cipher_test_enc|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|158|OpenSSL|cipher_test_enc|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|159|OpenSSL|cipher_test_enc|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|160|OpenSSL|cipher_test_enc|EVP_CIPHER_CTX_ctrl|d649c51a5388912277dffb56d921eb720db54be1|
|161|OpenSSL||do_X509_verify|bc42cf51c8b2a22282bb3cdf6303e230dc7b7873|
|162|OpenSSL||do_X509_REQ_verify|bc42cf51c8b2a22282bb3cdf6303e230dc7b7873|
|163|OpenSSL|print_stuff|SSL_export_keying_material|40649e36c4c0c9438f62e1bf2ccb983f6854c662|
|164|OpenSSL|print_connection_info|SSL_export_keying_material|40649e36c4c0c9438f62e1bf2ccb983f6854c662|
|165|OpenSSL|setup|BIO_set_indent|a9ed63f1d1d8993a8b30fc978ce09674f97f061d|
|166|OpenSSL|setup|BIO_set_prefix|ac6568ecc6050bc526adc6a7245835fd95d8dfed|
|167|OpenSSL|BN_generate_dsa_nonce|RAND_priv_bytes_ex|a8f4cdd70c9d9ebe4553d7a72c67f73eaf0c169d|
|168|OpenSSL|asn1_template_noexp_d2i|asn1_item_embed_d2i|7f608e4b1d9473258445144ba66216fb0e63aebe|
|169|OpenSSL|create_new_vfile|TXT_DB_write|aba9943fef8dcc8416ac9a219c97c616c1fd6344|
|170|OpenSSL|rsa_decryption_primitive_test|EVP_PKEY_decrypt|0650ac437b529274aca094c516a5a0127bbaf48c|
|171|OpenSSL|test_EVP_SM2|EVP_PKEY_decrypt|0650ac437b529274aca094c516a5a0127bbaf48c|
|172|OpenSSL|thread_shared_evp_pkey|EVP_PKEY_decrypt|0650ac437b529274aca094c516a5a0127bbaf48c|
|173|OpenSSL|file_try_read_msblob|ossl_do_blob_header|546b9f6b5cf6d0fde60aa37084eec1bb7d0fbc72|
|174|OpenSSL|read_to_eol|BIO_gets|7264068a15e7c4955efa25753430595a45caa16f|
|175|OpenSSL|dh_cms_set_shared_info|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|176|OpenSSL|pkey_type2param|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|177|OpenSSL|CMS_SignerInfo_sign|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|178|OpenSSL|get_legacy_evp_names|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|179|OpenSSL|OSSL_CRMF_pbm_new|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|180|OpenSSL|DH_KDF_X9_42|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|181|OpenSSL|spki2typespki_decode|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|182|OpenSSL|test_spki_aid|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|183|OpenSSL|test_explicit_EVP_MD_fetch_by_X509_ALGOR|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|184|OpenSSL|test_explicit_EVP_MD_fetch_by_X509_ALGOR|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|185|OpenSSL|test_explicit_EVP_CIPHER_fetch_by_X509_ALGOR|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|186|OpenSSL|test_explicit_EVP_CIPHER_fetch_by_X509_ALGOR|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|187|OpenSSL|SSL_load_client_CA_file_ex|BIO_read_filename|e3f0362407f6f40e413d6dcb35888514dbaed6f8|
|188|OpenSSL|SSL_add_file_cert_subjects_to_stack|BIO_read_filename|e3f0362407f6f40e413d6dcb35888514dbaed6f8|
|189|OpenSSL|cert_cb|BIO_read_filename|e3f0362407f6f40e413d6dcb35888514dbaed6f8|
|190|OpenSSL|cert_cb|BIO_read_filename|e3f0362407f6f40e413d6dcb35888514dbaed6f8|
|191|OpenSSL|cert_cb|BIO_read_filename|e3f0362407f6f40e413d6dcb35888514dbaed6f8|
|192|OpenSSL|dsaparam_main|EVP_PKEY_keygen_init|bf4ceeded1497c79e72fba4f9ff15febae58108d|
|193|OpenSSL|test_EC_keygen_with_enc|EVP_PKEY_keygen_init|bf4ceeded1497c79e72fba4f9ff15febae58108d|
|194|OpenSSL|test_EVP_SM2|EVP_PKEY_keygen_init|bf4ceeded1497c79e72fba4f9ff15febae58108d|
|195|OpenSSL|test_ecpub|EVP_PKEY_keygen_init|bf4ceeded1497c79e72fba4f9ff15febae58108d|
|196|OpenSSL|test_EVP_rsa_pss_with_keygen_bits|EVP_PKEY_keygen_init|bf4ceeded1497c79e72fba4f9ff15febae58108d|
|197|OpenSSL|gen_dh_key|EVP_PKEY_keygen_init|bf4ceeded1497c79e72fba4f9ff15febae58108d|
|198|OpenSSL|dhparam_main|EVP_PKEY_paramgen_init|6e0b05f3008a3f22105fd2bed9314b0bfa381f93|
|199|OpenSSL|dsa_paramgen|EVP_PKEY_paramgen_init|6e0b05f3008a3f22105fd2bed9314b0bfa381f93|
|200|OpenSSL|dsa_paramgen_test|EVP_PKEY_paramgen_init|6e0b05f3008a3f22105fd2bed9314b0bfa381f93|
|201|OpenSSL|test_EC_keygen_with_enc|EVP_PKEY_paramgen_init|6e0b05f3008a3f22105fd2bed9314b0bfa381f93|
|202|OpenSSL|demo_verify|EVP_DigestVerifyFinal|e2e5e72d5aec4d8d633cc5e9930f762da7973ab6|
|203|OpenSSL|test_EVP_DigestSignInit|EVP_DigestVerifyFinal|e2e5e72d5aec4d8d633cc5e9930f762da7973ab6|
|204|OpenSSL|test_EVP_DigestSignInit|EVP_DigestVerifyFinal|e2e5e72d5aec4d8d633cc5e9930f762da7973ab6|
|205|OpenSSL|test_EVP_DigestVerifyInit|EVP_DigestVerifyFinal|e2e5e72d5aec4d8d633cc5e9930f762da7973ab6|
|206|OpenSSL|test_EVP_DigestVerifyInit|EVP_DigestVerifyFinal|e2e5e72d5aec4d8d633cc5e9930f762da7973ab6|
|207|OpenSSL|test_EVP_SM2_verify|EVP_DigestVerifyFinal|e2e5e72d5aec4d8d633cc5e9930f762da7973ab6|
|208|OpenSSL|test_EVP_SM2|EVP_DigestVerifyFinal|e2e5e72d5aec4d8d633cc5e9930f762da7973ab6|
|209|OpenSSL|CMAC_Final|EVP_Cipher|6d774732517f1d63b7999c5691fc0bf046023faf|
|210|OpenSSL|ssl3_cbc_copy_mac|RAND_bytes_ex|c9007bda79291179ed2df31b3dfd9f1311102847|
|211|OpenSSL|dummy_provider_init|RAND_bytes_ex|c9007bda79291179ed2df31b3dfd9f1311102847|
|212|OpenSSL|execute_test_ktls_sendfile|RAND_bytes_ex|c9007bda79291179ed2df31b3dfd9f1311102847|
|213|OpenSSL|randomize_tls_group_id|RAND_bytes_ex|c9007bda79291179ed2df31b3dfd9f1311102847|
|214|OpenSSL|bn_rsa_fips186_4_find_aux_prob_prime|BN_check_prime|680827a15f12c3b37a6335fcb992555cf300730e|
|215|OpenSSL|ossl_bn_rsa_fips186_4_derive_prime|BN_check_prime|680827a15f12c3b37a6335fcb992555cf300730e|
|216|OpenSSL|X509_add_cert|X509_self_signed|64c428c35053a101a452c42d5d0a9a8342493606|
|217|OpenSSL|build_chain|X509_self_signed|64c428c35053a101a452c42d5d0a9a8342493606|
|218|OpenSSL|build_chain|X509_self_signed|64c428c35053a101a452c42d5d0a9a8342493606|
|219|OpenSSL|x509_name_ex_i2d|x509_name_canon|09235289c377ff998964bb6b074bb2a3ad768fd2|
|220|OpenSSL|EC_POINT_bn2point|BN_bn2binpad|098f2627c8d283a518a6e6e60e7893664c7510e0|
|221|OpenSSL|pkey_get_bn_bytes|BN_bn2binpad|098f2627c8d283a518a6e6e60e7893664c7510e0|
|222|OpenSSL|fbytes|BN_bn2binpad|098f2627c8d283a518a6e6e60e7893664c7510e0|
|223|OpenSSL|default_fixup_args|BN_bn2nativepad|944fcfc69d16dfd20decdd9cd105436f0043dbe0|
|224|OpenSSL|get_ulong_via_BN|BN_bn2nativepad|944fcfc69d16dfd20decdd9cd105436f0043dbe0|
|225|OpenSSL|test_print_key_using_pem|EVP_PKEY_print_private|8b7d7789dc4ea0de11331cb4045bcb03ab0864fc|
|226|OpenSSL|dsa_to_dh|EVP_PKEY_fromdata|d11cab47810715ba472070300b180944a1d93633|
|227|OpenSSL|new_cmac_key_int|EVP_PKEY_fromdata|d11cab47810715ba472070300b180944a1d93633|
|228|OpenSSL|get_dh_from_pg_bn|EVP_PKEY_fromdata|d11cab47810715ba472070300b180944a1d93633|
|229|OpenSSL|dsa_to_dh|EVP_PKEY_fromdata_init|5e199c356d09aca3b625b5ea16966b36d24b0201|
|230|OpenSSL|new_cmac_key_int|EVP_PKEY_fromdata_init|5e199c356d09aca3b625b5ea16966b36d24b0201|
|231|OpenSSL|get_dh_from_pg_bn|EVP_PKEY_fromdata_init|5e199c356d09aca3b625b5ea16966b36d24b0201|
|232|OpenSSL|dgst_main|BIO_get_md_ctx|59a3e7b29574ff45f62e825f6e9923f45060f142|
|233|OpenSSL|dgst_main|BIO_get_md_ctx|59a3e7b29574ff45f62e825f6e9923f45060f142|
|234|OpenSSL||BIO_free|d8f6c533cfcbcad350c9cfb2c112eb9f938ba83c|
|235|libzip|_zip_stat_win32|_zip_filetime_to_time_t|6dc79628531acf489d90940da859b2a381aac6de|




| Id | Codebase | API | Commit |
| -- | -------- | --- | ------ |
|0|OpenSSL|DES_is_weak_key|dd1f28427b375931fda45180619c8f5971cd6bca|
|1|OpenSSL|EVP_MD_get_size|dd1f28427b375931fda45180619c8f5971cd6bca|
|2|OpenSSL|EVP_MD_get_block_size|dd1f28427b375931fda45180619c8f5971cd6bca|
|3|OpenSSL|EVP_MD_CTX_get_size|dd1f28427b375931fda45180619c8f5971cd6bca|
|4|OpenSSL|EVP_MD_CTX_get_block_size|dd1f28427b375931fda45180619c8f5971cd6bca|
|5|OpenSSL|EVP_CIPHER_CTX_rand_key|dd1f28427b375931fda45180619c8f5971cd6bca|
|6|OpenSSL|EVP_PKEY_get1_encoded_public_key|dd1f28427b375931fda45180619c8f5971cd6bca|
|7|OpenSSL|EVP_VerifyFinal_ex|dd1f28427b375931fda45180619c8f5971cd6bca|
|8|OpenSSL|EVP_VerifyFinal|dd1f28427b375931fda45180619c8f5971cd6bca|
|9|OpenSSL|OCSP_request_onereq_count|dd1f28427b375931fda45180619c8f5971cd6bca|
|10|OpenSSL|RSA_sign|dd1f28427b375931fda45180619c8f5971cd6bca|
|11|OpenSSL|RSA_verify|dd1f28427b375931fda45180619c8f5971cd6bca|
|12|OpenSSL|SSL_CTX_set_generate_session_id|dd1f28427b375931fda45180619c8f5971cd6bca|
|13|OpenSSL|SSL_set_generate_session_id|dd1f28427b375931fda45180619c8f5971cd6bca|
|14|OpenSSL|SSL_has_matching_session_id|dd1f28427b375931fda45180619c8f5971cd6bca|
|15|OpenSSL|X509_NAME_entry_count|dd1f28427b375931fda45180619c8f5971cd6bca|
|16|OpenSSL|X509v3_get_ext_count|dd1f28427b375931fda45180619c8f5971cd6bca|
|17|OpenSSL|X509v3_get_ext_by_NID|dd1f28427b375931fda45180619c8f5971cd6bca|
|18|OpenSSL|i2d_re_X509_tbs|dd1f28427b375931fda45180619c8f5971cd6bca|
|19|OpenSSL|i2d_re_X509_CRL_tbs|dd1f28427b375931fda45180619c8f5971cd6bca|
|20|OpenSSL|i2d_re_X509_REQ_tbs|dd1f28427b375931fda45180619c8f5971cd6bca|
|21|OpenSSL|OBJ_obj2txt|2349d7ba57c9327290df6f7bc18b7f0c3976ca9e|
|22|OpenSSL|BN_bn2lebinpad|aedc5a819ee3f5267a7ec5c795b97481a1c63dc6|
|23|OpenSSL|BN_bn2nativepad|aedc5a819ee3f5267a7ec5c795b97481a1c63dc6|
|24|OpenSSL|DSAparams_print|8b7d7789dc4ea0de11331cb4045bcb03ab0864fc|
|25|OpenSSL|DSAparams_print_fp|8b7d7789dc4ea0de11331cb4045bcb03ab0864fc|
|26|OpenSSL|DSA_print|8b7d7789dc4ea0de11331cb4045bcb03ab0864fc|
|27|OpenSSL|DSA_print_fp|8b7d7789dc4ea0de11331cb4045bcb03ab0864fc|
|28|OpenSSL|ASN1_TYPE_get|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|29|OpenSSL|BIO_get_ktls_send|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|30|OpenSSL|BIO_get_ktls_recv|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|31|OpenSSL|BIO_reset|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|32|OpenSSL|BIO_flush|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|33|OpenSSL|BIO_eof|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|34|OpenSSL|BIO_set_close|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|35|OpenSSL|BIO_get_close|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|36|OpenSSL|BIO_get_buffer_num_lines|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|37|OpenSSL|BIO_set_read_buffer_size|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|38|OpenSSL|BIO_set_write_buffer_size|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|39|OpenSSL|BIO_set_buffer_size|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|40|OpenSSL|BIO_set_buffer_read_data|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|41|OpenSSL|BIO_set_cipher|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|42|OpenSSL|BIO_get_cipher_status|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|43|OpenSSL|BIO_get_cipher_ctx|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|44|OpenSSL|BIO_set_md|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|45|OpenSSL|BIO_get_md|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|46|OpenSSL|BIO_md_ctx|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|47|OpenSSL|BIO_set_prefix|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|48|OpenSSL|BIO_set_indent|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|49|OpenSSL|BIO_get_indent|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|50|OpenSSL|BIO_do_accept|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|51|OpenSSL|BIO_set_accept_name|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|52|OpenSSL|BIO_set_accept_port|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|53|OpenSSL|BIO_set_nbio_accept|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|54|OpenSSL|BIO_set_accept_bios|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|55|OpenSSL|BIO_set_accept_ip_family|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|56|OpenSSL|BIO_set_bind_mode|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|57|OpenSSL|BIO_get_accept_ip_family|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|58|OpenSSL|BIO_get_bind_mode|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|59|OpenSSL|BIO_set_conn_address|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|60|OpenSSL|BIO_set_conn_port|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|61|OpenSSL|BIO_set_conn_ip_family|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|62|OpenSSL|BIO_set_conn_hostname|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|63|OpenSSL|BIO_set_nbio|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|64|OpenSSL|BIO_do_connect|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|65|OpenSSL|BIO_set_fd|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|66|OpenSSL|BIO_get_fd|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|67|OpenSSL|BIO_seek|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|68|OpenSSL|BIO_tell|5001287c0dcd8ca4ffc564b360f86df79bba40c1|
|69|OpenSSL|ENGINE_register_complete|f1d077f1108b1bc2334350a4d53a46e29e082910|
|70|OpenSSL|ENGINE_register_all_complete|f1d077f1108b1bc2334350a4d53a46e29e082910|
|71|OpenSSL|ENGINE_free|f1d077f1108b1bc2334350a4d53a46e29e082910|
|72|OpenSSL|ERR_load_strings|f1d077f1108b1bc2334350a4d53a46e29e082910|
|73|OpenSSL|BIO_set_fp|9efdf4ad6b130aa4e206a8fd612539925c0b1e45|
|74|OpenSSL|BIO_get_fp|9efdf4ad6b130aa4e206a8fd612539925c0b1e45|
|75|OpenSSL|BIO_read_filename|9efdf4ad6b130aa4e206a8fd612539925c0b1e45|
|76|OpenSSL|BIO_write_filename|9efdf4ad6b130aa4e206a8fd612539925c0b1e45|
|77|OpenSSL|BIO_append_filename|9efdf4ad6b130aa4e206a8fd612539925c0b1e45|
|78|OpenSSL|BIO_rw_filename|9efdf4ad6b130aa4e206a8fd612539925c0b1e45|
|79|libwebsockets|lws_genrsa_hash_sign|433ad32216757547d321e87bdfef64771939f7ec|
|80|libwebsockets|lws_http_client_http_response|433ad32216757547d321e87bdfef64771939f7ec|
|81|libwebsockets|lws_http_transaction_completed|433ad32216757547d321e87bdfef64771939f7ec|
|82|libwebsockets|lws_x509_parse_from_pem|433ad32216757547d321e87bdfef64771939f7ec|
|83|libwebsockets|lws_tls_peer_cert_info|433ad32216757547d321e87bdfef64771939f7ec|
|84|libwebsockets|lws_tls_vhost_cert_info|433ad32216757547d321e87bdfef64771939f7ec|
|85|libwebsockets|lws_tls_acme_sni_cert_create|433ad32216757547d321e87bdfef64771939f7ec|
|86|libwebsockets|lws_genhash_size|433ad32216757547d321e87bdfef64771939f7ec|
|87|libwebsockets|lws_genhmac_size|433ad32216757547d321e87bdfef64771939f7ec|
|88|libwebsockets|lws_jws_parse_jose|433ad32216757547d321e87bdfef64771939f7ec|
|89|libwebsockets|lws_jwe_parse_jose|433ad32216757547d321e87bdfef64771939f7ec|
|90|libwebsockets|lws_jwk_strdup_meta|433ad32216757547d321e87bdfef64771939f7ec|
|91|libwebsockets|lws_jwk_generate|433ad32216757547d321e87bdfef64771939f7ec|
|92|libwebsockets|lws_jws_sig_confirm_compact|433ad32216757547d321e87bdfef64771939f7ec|
|93|libwebsockets|lws_jws_sig_confirm_compact_b64|433ad32216757547d321e87bdfef64771939f7ec|
|94|libwebsockets|lws_jws_sig_confirm|433ad32216757547d321e87bdfef64771939f7ec|
|95|libwebsockets|lws_jws_sign_from_b64|433ad32216757547d321e87bdfef64771939f7ec|
|96|libwebsockets|lws_jwt_sign_token_set_cookie|433ad32216757547d321e87bdfef64771939f7ec|
|97|libwebsockets|lws_jrpc_obj_parse|433ad32216757547d321e87bdfef64771939f7ec|
|98|libwebsockets|lws_genecdsa_hash_sign_jws|433ad32216757547d321e87bdfef64771939f7ec|
|99|libwebsockets|lws_write_numeric_address|433ad32216757547d321e87bdfef64771939f7ec|
|100|libwebsockets|lws_sa46_write_numeric_address|433ad32216757547d321e87bdfef64771939f7ec|
|101|libwebsockets|lws_sspc_client_connect|433ad32216757547d321e87bdfef64771939f7ec|
|102|libwebsockets|lws_sspc_set_metadata|433ad32216757547d321e87bdfef64771939f7ec|
