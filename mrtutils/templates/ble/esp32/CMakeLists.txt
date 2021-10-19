idf_component_register(SRCS 
                        "${obj.name.lower()}_gatt_server.c"
                        "interface/mrt_gatt_interface.c"
                        "interface/esp32_gatt_adapter.c"  
%for svc in obj.services:
                        "svc/${svc.prefix}_svc.c"
                        "app_${svc.prefix}_svc.c"
%endfor
               INCLUDE_DIRS interface svc)