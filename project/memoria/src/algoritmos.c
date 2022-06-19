#include "libstatic.h"
#include "memoria.h"
#include "serializado.h"
#include "utils-cliente.h"
#include "utils-servidor.h"
#include <commons/collections/dictionary.h>
#include <commons/string.h>

int obtener_y_asignar_marco_segun_algoritmo_de_reemplazo( int pid, t_entrada_tabla_segundo_nivel* entrada_segundo_nivel_solicitada_para_acceder){
  t_list* marcos_asignados_al_proceso = obtener_marcos_asignados_a_este_proceso(pid);
  t_entrada_tabla_segundo_nivel* entrada_segundo_nivel_victima = malloc(sizeof(t_entrada_tabla_segundo_nivel));
  int numero_marco_elegido = 0;

  if(algoritmo_reemplazo_cargado_es("CLOCK")){
    // TODO: cambiar bit de presencia en 0 de la victima, asignarle el marco a la nueva, sacarle el frame a la entrada en la TP
    entrada_segundo_nivel_victima = ejecutar_clock(marcos_asignados_al_proceso, entrada_segundo_nivel_solicitada_para_acceder);

    // TODO: REALIZAR EL REEMPLAZO ENTRE LA ENTRADA A REEMPLAZAR Y LA ENTRADA REFERENCIADA

  } else {
    // marco_victima = ejecutar_clock_modificado(marcos_involucrados, entrada);
  }

  numero_marco_elegido = entrada_segundo_nivel_victima->num_marco;

  // le asignamos el marco de la página víctima y la cargamos a memoria principal
  entrada_segundo_nivel_solicitada_para_acceder->num_marco = numero_marco_elegido;
  entrada_segundo_nivel_solicitada_para_acceder->bit_presencia = 1;

  // TODO: falta pasar la entrada a swap
  entrada_segundo_nivel_victima->bit_presencia = 0;

  return numero_marco_elegido;
}

// útil para diferenciar el criterio que elige este algoritmo de reemplazo como víctima
bool es_victima_segun_algoritmo_clock(t_entrada_tabla_segundo_nivel* entrada_elegida){
  return entrada_elegida->bit_uso == 0;
}

void algoritmo_clock_actualizar_puntero(t_marco* marco_seleccionado, t_marco* proximo_marco_seleccionado){
  marco_seleccionado->apuntado_por_puntero_de_clock = false;
  proximo_marco_seleccionado->apuntado_por_puntero_de_clock = true;
}

// TODO: iterar a partir del marco al que apunte el puntero de clock
t_entrada_tabla_segundo_nivel* entrada_victima_elegida_por_algoritmo_clock(t_list* marcos_asignados, t_entrada_tabla_segundo_nivel* entrada_solicitada_para_acceder){
  t_entrada_tabla_segundo_nivel* entrada_victima_elegida = malloc(sizeof(t_entrada_tabla_segundo_nivel));
  bool victima_encontrada = false;

  // iterar sobre la cola circular hasta que encuentre una pagina víctima
  for(int posicion_marco_leido = 0; !victima_encontrada ; posicion_marco_leido++){
    t_marco* marco_seleccionado = list_get(marcos_asignados, posicion_marco_leido);
    t_marco* proximo_marco_seleccionado = list_get(marcos_asignados, posicion_marco_leido + 1);
    t_entrada_tabla_segundo_nivel* entrada_asignada_al_marco = marco_seleccionado->entrada_segundo_nivel;

    if(!es_victima_segun_algoritmo_clock(entrada_asignada_al_marco)){
      // le damos otra oportunidad
      entrada_asignada_al_marco->bit_uso = 0;
    }
    else{
      entrada_victima_elegida = entrada_asignada_al_marco;

      // corta el flujo de iteración del for, no usamos un break/return para poder actualizar el puntero del clock
      victima_encontrada = true;
    }

    algoritmo_clock_actualizar_puntero(marco_seleccionado, proximo_marco_seleccionado);

    // esto facilita el uso del algoritmo de reemplazo clock, que cada marco guarde la entrada a la que apunta
    marco_seleccionado->entrada_segundo_nivel = entrada_solicitada_para_acceder;

    // volvemos al principio de la cola circular, el for seguirá iterando
    if(posicion_marco_leido > list_size(marcos_asignados)) posicion_marco_leido = 0;
  }

  return entrada_victima_elegida;
}

t_entrada_tabla_segundo_nivel* ejecutar_clock(t_list* marcos_asignados, t_entrada_tabla_segundo_nivel* entrada_solicitada_para_acceder) {
  t_entrada_tabla_segundo_nivel* entrada_victima = malloc(sizeof(t_entrada_tabla_segundo_nivel));

  while (puntero_clock < list_size(marcos_asignados)) {
    t_marco_asignado* marco_asignado_a_entrada = list_get(marcos_asignados, puntero_clock);

    if (marco_asignado_a_entrada->entrada->bit_uso != 0) {
      marco_asignado_a_entrada->entrada->bit_uso = 0;
    }
    else {
      entrada_victima = marco_asignado_a_entrada->entrada;
      puntero_clock++;
      break;
    }

    if (puntero_clock + 1 == list_size(marcos_asignados)) {
      puntero_clock = 0;
      continue;
    }
    puntero_clock++;
  }

  return entrada_victima;
}
