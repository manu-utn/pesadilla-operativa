#include "serializado.h"
#include "libstatic.h"
#include <commons/collections/list.h>

void* serializar_paquete(t_paquete* paquete) {
  // int size_paquete = sizeof(int) + sizeof(int) + paquete->buffer->size;
  int size_paquete = get_paquete_size(paquete);
  void* paquete_serializado = NULL;

  paquete_serializado = malloc(size_paquete); // TODO: need free (3)
  int offset = 0;
  memcpy(paquete_serializado + offset, &(paquete->codigo_operacion), sizeof(int));

  offset += sizeof(int);
  memcpy(paquete_serializado + offset, &(paquete->buffer->size), sizeof(int));

  offset += sizeof(int);
  memcpy(paquete_serializado + offset, paquete->buffer->stream, paquete->buffer->size);

  return paquete_serializado;
}

void** deserializar_paquete(t_paquete* paquete_serializado) {
  int offset, size_mensaje;
  void** mensajes = NULL;

  offset = 0;

  size_mensaje = 0;
  for (int index = 0, n = 1; offset < paquete_serializado->buffer->size; n++, index++) {
    mensajes = realloc(mensajes, sizeof(t_buffer) * n);

    t_buffer* mensaje = empty_buffer();

    size_mensaje = *(int*)(paquete_serializado->buffer->stream + offset);
    mensaje->size = size_mensaje;

    offset += sizeof(int);
    mensaje->stream = malloc(size_mensaje);
    memcpy(mensaje->stream, paquete_serializado->buffer->stream + offset, size_mensaje);

    mensajes[index] = (t_buffer*)mensaje;
    offset += size_mensaje;
    mensajes[index + 1] = NULL;
  }

  return mensajes;
}

void paquete_add_pcb(t_paquete* paquete, t_pcb* pcb) {
  int offset;
  int paquete_size = sizeof(int) * 5 + sizeof(t_pcb_estado);
  paquete->buffer->stream = malloc(paquete_size);

  offset = 0, memcpy(paquete->buffer->stream + offset, &(pcb->socket), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->pid), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->tamanio), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->estimacion_rafaga), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->program_counter), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->estado), sizeof(t_pcb_estado));
  offset += sizeof(t_pcb_estado);

  paquete->buffer->size = offset;
  for (int i = 0; i < list_size(pcb->instrucciones); i++) {
    t_instruccion* instruccion = list_get(pcb->instrucciones, i);

    int identificador_longitud = strlen(instruccion->identificador) + 1;
    int identificador_size = identificador_longitud * sizeof(char);

    int params_longitud = strlen(instruccion->params) + 1;
    int params_size = params_longitud * sizeof(char);

    int instruccion_size = identificador_size + params_size + sizeof(int) + sizeof(int);

    paquete->buffer->stream = realloc(paquete->buffer->stream, offset + instruccion_size);
    paquete_add_instruccion(paquete, instruccion);

    offset += instruccion_size;
  }
  paquete->buffer->size = offset;
}

void paquete_add_instruccion(t_paquete* paquete, t_instruccion* instruccion) {
  int identificador_longitud = strlen(instruccion->identificador) + 1;
  int identificador_size = identificador_longitud * sizeof(char);

  int params_longitud = strlen(instruccion->params) + 1;
  int params_size = params_longitud * sizeof(char);

  int instruccion_size = identificador_size + params_size + sizeof(int) + sizeof(int);

  int offset = 0;

  if (paquete->buffer->stream == NULL) {
    paquete->buffer->stream = malloc(instruccion_size);
  } else {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + instruccion_size);
    offset = paquete->buffer->size;
  }

  memcpy(paquete->buffer->stream + offset, &identificador_size, sizeof(int));

  offset += sizeof(int);
  memcpy(paquete->buffer->stream + offset, instruccion->identificador, identificador_size);

  offset += identificador_size;
  memcpy(paquete->buffer->stream + offset, &params_size, sizeof(int));

  offset += sizeof(int);
  memcpy(paquete->buffer->stream + offset, instruccion->params, params_size);

  paquete->buffer->size = paquete->buffer->size + instruccion_size;

  offset += params_size;
}

t_list* paquete_obtener_instrucciones(t_paquete* paquete_serializado) {
  int offset = 0;
  t_list* lista = list_create();

  while (offset < paquete_serializado->buffer->size) {
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    int identificador_size = 0, params_size = 0;

    identificador_size = *(int*)(paquete_serializado->buffer->stream + offset);
    instruccion->identificador = malloc(identificador_size);

    offset += sizeof(int);
    memcpy(instruccion->identificador, paquete_serializado->buffer->stream + offset, identificador_size);

    offset += identificador_size;
    params_size = *(int*)(paquete_serializado->buffer->stream + offset);
    instruccion->params = malloc(params_size);

    offset += sizeof(int);
    memcpy(instruccion->params, paquete_serializado->buffer->stream + offset, params_size);

    list_add(lista, instruccion);

    offset += params_size;
  }

  return lista;
}

t_pcb* paquete_obtener_pcb(t_paquete* paquete_serializado) {
  int offset = 0;

  t_pcb* pcb = malloc(sizeof(t_pcb));

  memcpy(&(pcb->socket), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->pid), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->tamanio), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->estimacion_rafaga), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->program_counter), paquete_serializado->buffer->stream + offset, sizeof(int));

  offset += sizeof(int);
  memcpy(&(pcb->estado), paquete_serializado->buffer->stream + offset, sizeof(t_pcb_estado));

  offset += sizeof(t_pcb_estado);

  int instrucciones_size = paquete_serializado->buffer->size - offset;

  t_paquete* paquete_con_instrucciones = paquete_create();
  paquete_con_instrucciones->buffer->stream = malloc(instrucciones_size);
  paquete_con_instrucciones->buffer->size = instrucciones_size;

  memcpy(paquete_con_instrucciones->buffer->stream, paquete_serializado->buffer->stream + offset, instrucciones_size);

  pcb->instrucciones = paquete_obtener_instrucciones(paquete_con_instrucciones);

  paquete_destroy(paquete_con_instrucciones);

  return pcb;
}

t_mensaje_handshake_cpu_memoria* paquete_obtener_mensaje_handshake(t_paquete* paquete_serializado) {
  int offset = 0;

  t_mensaje_handshake_cpu_memoria* mensaje = malloc(sizeof(t_mensaje_handshake_cpu_memoria));

  memcpy(&(mensaje->socket), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);
  memcpy(&(mensaje->size_mensaje), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);
  memcpy(&(mensaje->mensaje_handshake), paquete_serializado->buffer->stream + offset, mensaje->size_mensaje);
  offset += mensaje->size_mensaje;

  return mensaje;
}

void paquete_add_mensaje_handshake(t_paquete* paquete_serializado, t_mensaje_handshake_cpu_memoria* mensaje_handshake) {
  int offset = 0;

  int size_paquete = sizeof(int) + strlen(mensaje_handshake->mensaje_handshake) + 1;
  paquete_serializado->buffer->stream = malloc(size_paquete);
  mensaje_handshake->size_mensaje = strlen(mensaje_handshake->mensaje_handshake);

  memcpy(paquete_serializado->buffer->stream, &(mensaje_handshake->size_mensaje), sizeof(int));

  offset += sizeof(int);
  memcpy(paquete_serializado->buffer->stream + offset,
         mensaje_handshake->mensaje_handshake,
         strlen(mensaje_handshake->mensaje_handshake));

  paquete_serializado->buffer->size = size_paquete;

  offset += strlen(mensaje_handshake->mensaje_handshake);
}


void paquete_add_operacion_read(t_paquete* paquete_serializado, t_operacion_read* read) {
  int offset = 0;

  int size_paquete = sizeof(uint32_t) * 2;
  paquete_serializado->buffer->stream = malloc(size_paquete);

  memcpy(paquete_serializado->buffer->stream, &(read->socket), sizeof(uint32_t));
  offset += sizeof(uint32_t);
  memcpy(paquete_serializado->buffer->stream, &(read->direccion_logica), sizeof(uint32_t));
  paquete_serializado->buffer->size = size_paquete;

  offset += sizeof(read->direccion_logica);
}

void paquete_add_solicitud_tabla_segundo_nivel(t_paquete* paquete_serializado, t_solicitud_segunda_tabla* read) {
  int offset = 0;

  int size_paquete = sizeof(int) * 3;
  paquete_serializado->buffer->stream = malloc(size_paquete);

  memcpy(paquete_serializado->buffer->stream, &(read->socket), sizeof(int));
  offset += sizeof(int);
  memcpy(paquete_serializado->buffer->stream, &(read->num_tabla_primer_nivel), sizeof(int));
  offset += sizeof(int);
  memcpy(paquete_serializado->buffer->stream, &(read->entrada_primer_nivel), sizeof(int));
  offset += sizeof(int);
  paquete_serializado->buffer->size = size_paquete;
  // offset += sizeof(read->direccion_logica);
}


t_operacion_read* paquete_obtener_operacion_read(t_paquete* paquete_serializado) {
  int offset = 0;

  t_operacion_read* read = malloc(sizeof(t_operacion_read));
  memcpy(&(read->socket), paquete_serializado->buffer->stream + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  memcpy(&(read->direccion_logica), paquete_serializado->buffer->stream + offset, sizeof(uint32_t));


  return read;
}


t_solicitud_segunda_tabla* paquete_obtener_solicitud_tabla_segundo_nivel(t_paquete* paquete_serializado) {
  int offset = 0;

  t_solicitud_segunda_tabla* read = malloc(sizeof(t_solicitud_segunda_tabla));
  memcpy(&(read->socket), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);
  memcpy(&(read->num_tabla_primer_nivel), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);
  memcpy(&(read->entrada_primer_nivel), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);

  return read;
}


void paquete_add_operacion_IO(t_paquete* paquete, t_pcb* pcb, int tiempo_bloqueo) { // es un add_pcb + el campo bloqueo

  int offset;
  int paquete_size = sizeof(int) * 6 + sizeof(t_pcb_estado) + 1;
  paquete->buffer->stream = malloc(paquete_size);

  offset = 0, memcpy(paquete->buffer->stream + offset, &(pcb->socket), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->pid), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->tamanio), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->estimacion_rafaga), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->program_counter), sizeof(int));
  offset += sizeof(int), memcpy(paquete->buffer->stream + offset, &(pcb->estado), sizeof(t_pcb_estado));
  offset += sizeof(t_pcb_estado), memcpy(paquete->buffer->stream + offset, &tiempo_bloqueo, sizeof(int));
  offset += sizeof(int);

  paquete->buffer->size = offset;
  for (int i = 0; i < list_size(pcb->instrucciones); i++) {
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));

    instruccion = list_get(pcb->instrucciones, i);

    int identificador_longitud = strlen(instruccion->identificador) + 1;
    int identificador_size = identificador_longitud * sizeof(char);

    int params_longitud = strlen(instruccion->params) + 1;
    int params_size = params_longitud * sizeof(char);

    int instruccion_size = identificador_size + params_size + sizeof(int) + sizeof(int);

    paquete->buffer->stream = realloc(paquete->buffer->stream, offset + instruccion_size);
    paquete_add_instruccion(paquete, instruccion);

    offset += instruccion_size;
    free(instruccion);
  }
  paquete->buffer->size = offset;
}

void paquete_add_respuesta_operacion_read(t_paquete* paquete, t_respuesta_operacion_read* respuesta_read) {
  int offset = 0;
  int paquete_size = sizeof(int);
  paquete->buffer->stream = malloc(paquete_size);

  memcpy(paquete->buffer->stream + offset, &respuesta_read->socket, sizeof(int));
  offset += sizeof(int);
  memcpy(paquete->buffer->stream + offset, &respuesta_read->valor_buscado, sizeof(int));
  offset += sizeof(int);

  paquete->buffer->size = offset;
}

t_respuesta_operacion_read* obtener_respuesta_read(t_paquete* paquete_serializado) {
  int offset = 4;
  t_respuesta_operacion_read* read = malloc(sizeof(t_respuesta_operacion_read));
  memcpy(&(read->valor_buscado), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);

  return read;
}

t_respuesta_solicitud_segunda_tabla* obtener_respuesta_solicitud_tabla_segundo_nivel(t_paquete* paquete_serializado) {
  int offset = 0;

  t_respuesta_solicitud_segunda_tabla* read = malloc(sizeof(t_respuesta_solicitud_segunda_tabla));
  memcpy(&(read->socket), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);
  memcpy(&(read->entrada_segundo_nivel), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);

  return read;
}


void paquete_add_solicitud_marco(t_paquete* paquete_serializado, t_solicitud_marco* solicitud_marco) {
  int offset = 0;

  int size_paquete = sizeof(int) * 3;
  paquete_serializado->buffer->stream = malloc(size_paquete);

  memcpy(paquete_serializado->buffer->stream, &(solicitud_marco->socket), sizeof(int));
  offset += sizeof(int);
  memcpy(paquete_serializado->buffer->stream, &(solicitud_marco->num_tabla_segundo_nivel), sizeof(int));
  offset += sizeof(int);
  memcpy(paquete_serializado->buffer->stream, &(solicitud_marco->entrada_segundo_nivel), sizeof(int));
  offset += sizeof(int);
  paquete_serializado->buffer->size = size_paquete;
  // offset += sizeof(read->direccion_logica);
}


t_respuesta_solicitud_marco* obtener_respuesta_solicitud_marco(t_paquete* paquete_serializado) {
  int offset = 0;

  t_respuesta_solicitud_marco* solicitud_marco = malloc(sizeof(t_respuesta_solicitud_marco));
  memcpy(&(solicitud_marco->num_marco), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);
  return solicitud_marco;
}

void paquete_add_solicitud_dato_fisico(t_paquete* paquete_serializado, t_solicitud_dato_fisico* solicitud_dato_fisico) {
  int offset = 0;

  int size_paquete = sizeof(int) * 3;
  paquete_serializado->buffer->stream = malloc(size_paquete);

  memcpy(paquete_serializado->buffer->stream, &(solicitud_dato_fisico->socket), sizeof(int));
  offset += sizeof(int);
  memcpy(paquete_serializado->buffer->stream, &(solicitud_dato_fisico->dir_fisica), sizeof(int));
  offset += sizeof(int);
  paquete_serializado->buffer->size = size_paquete;
  // offset += sizeof(read->direccion_logica);
}


t_respuesta_dato_fisico* obtener_respuesta_solicitud_dato_fisico(t_paquete* paquete_serializado) {
  int offset = 0;

  t_respuesta_dato_fisico* respuesta_dato = malloc(sizeof(t_respuesta_dato_fisico));
  memcpy(&(respuesta_dato->size_dato), paquete_serializado->buffer->stream + offset, sizeof(int));
  offset += sizeof(int);
  memcpy(respuesta_dato->dato_buscado, paquete_serializado->buffer->stream + offset, respuesta_dato->size_dato);
  offset += respuesta_dato->size_dato;
  return respuesta_dato;
}