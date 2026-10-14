/*
 * zbnode_ui.h
 * Giao dien TFT ILI9341 320x240 cho ZbNode Zigbee
 *
 * Nut bam (ngat ngoai):
 * PA0 = BTN1  ->  UI_Button(0)   BACK / MENU
 * PA1 = BTN2  ->  UI_Button(1)   LEN  / NODE TRUOC
 * PA2 = BTN3  ->  UI_Button(2)   XUONG / NODE TIEP
 * PA3 = BTN4  ->  UI_Button(3)   CHON / VAO / NEXT PANEL
 */

#ifndef ZBNODE_UI_H_
#define ZBNODE_UI_H_

#include "zigbee.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * View enum
 * ============================================================ */
#define UI_VIEW_OV  0   /* Tong hop */
#define UI_VIEW_DT  1   /* Chi tiet */

/* ============================================================
 * API
 * ============================================================ */

void UI_SetNodeThreshold(uint16_t addr, int16_t temp_th, int16_t curr_th);
void UI_Alarm_Tick(void);
/*
 * Goi 1 lan sau TFT_Init():
 * nodes  = mang pointer toi ZbNode_t (toi da 4 node)
 * count  = so luong node thuc su
 */
void UI_Init(ZbNode_t **nodes, uint8_t count);

/*
 * Ve lai toan bo giao dien (goi sau khi data thay doi,
 * hoac dinh ky tu vong chinh).
 */
void UI_Render(void);

/*
 * Xu ly nut bam:
 * btn = 0 (PA0/BTN1), 1 (PA1/BTN2), 2 (PA2/BTN3), 3 (PA3/BTN4)
 */
void UI_Button(uint8_t btn);

/*
 * Tien ich: goi khi ZbNode data cap nhat tu Zigbee stack.
 */
void UI_Update(void);

#endif /* ZBNODE_UI_H_ */