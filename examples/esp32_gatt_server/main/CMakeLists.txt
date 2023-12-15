file(GLOB_RECURSE SRC_FILES
    *.cc
    )

idf_component_register(SRCS ${SRC_FILES}
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES 
                       freertos 
                       blefacade
                       nvs_flash 
                       bt)
