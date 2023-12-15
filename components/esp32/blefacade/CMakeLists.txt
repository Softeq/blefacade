

idf_component_register(SRCS "esp32_backend.cc"
                       INCLUDE_DIRS "." "${BLEFACADE_ORIGIN_DIR}/include"
                       PRIV_INCLUDE_DIRS 
                       PRIV_REQUIRES bt)
