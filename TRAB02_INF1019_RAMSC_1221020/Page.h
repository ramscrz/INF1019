/* Trabalho 02: Simulando Memória Virtual e Substituição de Páginas LFU
 * INF1019 - Sistemas de Computação
 * 11/12/2017
 * Autor: Rafael AZEVEDO MOSCOSO SILVA CRUZ
 * Arquivo Page.h
 * PUC-Rio
 */

#ifndef PAGE
#define PAGE

typedef struct pageFrameTable PageFrameTable;

typedef struct page Page;
typedef struct pageTable PageTable;

typedef enum {

	ACCESSMODE_R,
	ACCESSMODE_W

} ACCESS_MODE;

typedef enum {

	PAGE_NOT_MODIFIED,
	PAGE_MODIFIED

} MODIFIED_CONTROL_BIT;

typedef enum {

	PAGE_NOT_FRAMED,
	PAGE_FRAMED

} FRAMED_CONTROL_BIT;

typedef enum {

	PAGE_NOT_NEW,
	PAGE_NEW

} NEW_CONTROL_BIT;

#endif

