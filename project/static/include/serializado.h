#ifndef UTILS_SERIALIZADO_H_
#define UTILS_SERIALIZADO_H_

#include <commons/config.h>
#include <commons/log.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "libstatic.h"

t_log *logger;

void *serializar_paquete(t_paquete *paquete);
t_list* deserializar_paquete(t_paquete *paquete_serializado);
void paquete_add_instruccion(t_paquete *paquete, t_instruccion *instruccion);
t_list *paquete_obtener_instrucciones(t_paquete *paquete_serializado);
void paquete_add_pcb(t_paquete *paquete, t_pcb *pcb);
t_pcb *paquete_obtener_pcb(t_paquete *paquete_serializado);

void paquete_add_mensaje_handshake(t_paquete* paquete_serializado, t_mensaje_handshake_cpu_memoria* mensahe_handshake);
void paquete_add_respuesta_operacion_read(t_paquete *paquete, t_respuesta_operacion_read *respuesta_read);
t_operacion_read *paquete_obtener_operacion_read(t_paquete *paquete_serializado);
void paquete_add_operacion_IO(t_paquete *paquete, t_pcb *pcb, int tiempo_bloqueo);
void paquete_add_operacion_read(t_paquete *paquete_serializado, t_operacion_read *read);
t_respuesta_operacion_read *obtener_respuesta_read(t_paquete *paquete_serializado);
#endif
