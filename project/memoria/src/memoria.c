#include "memoria.h"
#include "libstatic.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"

void* reservar_memoria_inicial(int size_memoria_total) {
  void* memoria_total = malloc(size_memoria_total);

  memset(memoria_total, 0, size_memoria_total);

  return memoria_total;
}

void* escuchar_conexiones() {
  estado_conexion_memoria = true;
  char* ip = config_get_string_value(config, "IP_ESCUCHA");
  char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA");
  int socket_kernel = iniciar_servidor(ip, puerto);

  while (estado_conexion_memoria) {
    int cliente_fd = esperar_cliente(socket_kernel);

    if (cliente_fd != -1) {
      t_paquete* paquete = paquete_create();
      t_buffer* mensaje = crear_mensaje("Conexión aceptada por MEMORIA");

      paquete_cambiar_mensaje(paquete, mensaje), enviar_mensaje(cliente_fd, paquete);
      // paquete_add_mensaje(paquete, mensaje);
    }

    pthread_t th;
    pthread_create(&th, NULL, manejar_nueva_conexion, &cliente_fd), pthread_detach(th);
  }
  free(ip);
  free(puerto);
  pthread_exit(NULL);
}
void* manejar_nueva_conexion(void* args) {
  int socket_cliente = *(int*)args;
  estado_conexion_con_cliente = true;

  while (estado_conexion_con_cliente) {
    int codigo_operacion = recibir_operacion(socket_cliente);

    switch (codigo_operacion) {
      case MENSAJE_HANDSHAKE: {
        log_info(logger, "Handshake establecido...");
        // t_paquete* paquete = recibir_paquete(cliente_fd);
        // t_mensaje_handshake_cpu_memoria* mensaje = paquete_obtener_mensaje_handshake(paquete);

      } break;
      case READ: {
        log_info(logger, "Comenzando operacion READ...");
        log_info(logger, "HOLLAAAAAA");
        t_paquete* paquete = recibir_paquete(socket_cliente);
        t_operacion_read* read = paquete_obtener_operacion_read(paquete);

        // PROCESO EL VALOR ENVIADO POR CPU, POR AHORA HARDCODEO UN VALOR PARA PROBAR LA CONEXION

        t_respuesta_operacion_read* respuesta_read = malloc(sizeof(t_respuesta_operacion_read));
        respuesta_read->valor_buscado = 20314;
        t_paquete* paquete_con_respuesta = paquete_create();
        paquete_add_respuesta_operacion_read(paquete_con_respuesta, respuesta_read);
        enviar_operacion_read(read->socket, paquete_con_respuesta);
        // DESCOMENTAR PARA RESOLVER SEG FAULT
        paquete_destroy(paquete_con_respuesta);

        free(respuesta_read);
      }
      case -1: {
        log_info(logger, "el cliente se desconecto");
        estado_conexion_con_cliente = false;
        break;
      }
      default:
        log_warning(logger, "Operacion desconocida. No quieras meter la pata");
        break;
    }
  }
  pthread_exit(NULL);
}
