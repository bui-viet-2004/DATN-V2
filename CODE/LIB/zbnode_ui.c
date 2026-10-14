/*
 * zbnode_ui.c
 * Giao dien TFT ILI9341 (320x240, landscape) cho ZbNode
 * 4 nut: PA0=BTN1(BACK), PA1=BTN2(LEN), PA2=BTN3(XUONG), PA3=BTN4(CHON/VAO)
 */

#include "zbnode_ui.h"
#include "TFT_ILI9341.h"
#include <stdio.h>
#include <string.h>
#include "DHT.h"

/* ============================================================
 * COLOR PALETTE (RGB565)
 * ============================================================ */
#define C_BLACK     0x0000
#define C_WHITE     0xFFFF
#define C_BG        0x0000  
#define C_HDR_BG    0x0011  
#define C_HDR_LINE  0x0198  
#define C_PANEL_BG  0x0001  
#define C_PANEL_BD  0x0111  
#define C_CYAN      0x07FF  
#define C_BLUE_LT   0x245F  
#define C_GREEN     0x07E0  
#define C_RED       0xF800
#define C_ORANGE    0xFC00  
#define C_DARK_GRN  0x0320  
#define C_DARK_RED  0x4000  
#define C_GRAY      0x0228  
#define C_YELLOW    0xFFE0
#define C_SB_OK_BG  0x0300  
#define C_SB_ERR_BG 0xA000  

/* ============================================================
 * LAYOUT CONSTANTS (320x240)
 * ============================================================ */
#define HDR_H       16
#define SB_H        18
#define BODY_Y      (HDR_H)
#define BODY_H      (TFT_HEIGHT - HDR_H - SB_H)   /* 206 */
#define BODY_YEND   (HDR_H + BODY_H)               /* 222 */

/* Overview left panel */
#define OV_LEFT_W   74
#define OV_LEFT_X   2
#define OV_GRID_X   (OV_LEFT_X + OV_LEFT_W + 2)   /* 78 */
#define OV_GRID_W   (TFT_WIDTH - OV_GRID_X - 2)   /* 240 */

/* Detail side panel */
#define DT_SIDE_W   76
#define DT_SIDE_X   (TFT_WIDTH - DT_SIDE_W - 2)   /* 242 */
#define DT_PANEL_W  (DT_SIDE_X - 4)               /* 238 */

#define MAX_NODES   4

/* ============================================================
 * GLOBALS
 * ============================================================ */
uint8_t  ui_view      = UI_VIEW_OV;
static uint8_t  ui_view_prev = 255; /* Lưu cấu hình trang trước đó để kiểm tra chuyển mạch */
uint8_t  ui_selIdx    = 0;
static uint8_t  ui_dtPanel   = 0;   /* 0=NHIET, 1=DONG */
static uint8_t  ui_nodeCount = 0;
ZbNode_t *ui_nodes[MAX_NODES];

static int16_t thresh_temp[MAX_NODES] = {50, 50, 50, 50}; 
static int16_t thresh_curr[MAX_NODES] = {50, 50, 50, 50}; 

// Tìm vị trí máy dựa vào địa chỉ
static int8_t find_node_index(uint16_t addr) {
    for (uint8_t i = 0; i < ui_nodeCount; i++) {
        if (ui_nodes[i]->addr == addr) return i;
    }
    return -1; 
}

uint8_t global_hw_error = 0;

/* ============================================================
 * HELPER UTILS
 * ============================================================ */
static uint16_t lqi_color(uint8_t lqi)
{
    if (lqi < 50)  return C_RED;
    if (lqi < 100) return C_ORANGE;
    return C_GREEN;
}

/* Hàm kiểm tra màu sử dụng giá trị thực so sánh với ngưỡng số nguyên */
static uint16_t val_color_f(float v, int16_t th)
{
    if (v > (float)th)             return C_RED;
    if (v > ((float)th * 0.85f))   return C_ORANGE; 
    return C_GREEN;
}

/* Chỉ đếm lỗi của Nhiệt độ và Dòng điện */
static uint8_t node_err_count(ZbNode_t *n, uint8_t idx) {
    if (!n->online) return 1;
    uint8_t c = 0;
    if (n->lqi < 50) c++;
    if (n->temperature > (float)thresh_temp[idx]) c++;
    if (n->voltage_ep2 > (float)thresh_curr[idx]) c++;
    return c;
}

static void draw_label_val(uint16_t x, uint16_t y, const char *lbl, const char *val, uint16_t vcol)
{
    TFT_DrawString(x,     y, lbl, C_GRAY,  C_PANEL_BG, 1);
    TFT_DrawString(x+38,  y, val, vcol,    C_PANEL_BG, 1);
}

static void draw_hsep(uint16_t x, uint16_t y, uint16_t w)
{
    TFT_DrawHLine(x, y, w, C_PANEL_BD);
}

static void fill_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bg, uint16_t border)
{
    TFT_FillRect(x, y, w, h, bg);
    TFT_DrawRect(x, y, w, h, border);
}

/* ============================================================
 * HEADER
 * ============================================================ */
static void render_header(void)
{
    TFT_FillRect(0, 0, TFT_WIDTH, HDR_H, C_HDR_BG);
    TFT_DrawHLine(0, HDR_H - 1, TFT_WIDTH, C_HDR_LINE);

    const char *title = (ui_view == UI_VIEW_OV) ? "TONG HOP" : "CHI TIET";
    TFT_DrawString(4, 4, title, C_CYAN, C_HDR_BG, 1);

    char buf[24];
    if (ui_nodeCount > 0)
    {
        snprintf(buf, sizeof(buf), "N%d/%d", ui_selIdx + 1, ui_nodeCount);
        TFT_DrawString(TFT_WIDTH - 7*6 - 2, 4, buf, C_BLUE_LT, C_HDR_BG, 1);
    }
}

/* ============================================================
 * STATUS BAR
 * ============================================================ */
static void render_statusbar(uint8_t has_err, const char *msg)
{
    uint16_t bg  = has_err ? C_SB_ERR_BG : C_SB_OK_BG;
    uint16_t col = has_err ? C_WHITE : C_GREEN;

    TFT_FillRect(0, BODY_YEND, TFT_WIDTH, SB_H, bg);
    TFT_DrawHLine(0, BODY_YEND, TFT_WIDTH, C_HDR_LINE);

    if (!has_err)
    {
        TFT_DrawString(4, BODY_YEND + 5, "HE THONG ON DINH", C_GREEN, bg, 1);
    }
    else
    {
        TFT_DrawString(4, BODY_YEND + 5, msg, col, bg, 1);
    }
}

/* ============================================================
 * OVERVIEW – LEFT PANEL
 * ============================================================ */
static void render_ov_left(void)
{
    uint16_t x = OV_LEFT_X;
    uint16_t y = BODY_Y + 2;

    uint8_t online = 0, errtot = 0;
    for (uint8_t i = 0; i < ui_nodeCount; i++)
    {
        ZbNode_t *n = ui_nodes[i];
        if (n->online) online++;
        errtot += node_err_count(n, i);
    }

    TFT_DrawRect(x, y, OV_LEFT_W, 60, C_PANEL_BD);

    TFT_DrawString(x+2, y+2, "THONG KE", C_CYAN, C_PANEL_BG, 1);
    draw_hsep(x+1, y+10, OV_LEFT_W-2);

    char buf[16];
    uint16_t ry = y + 13;
    
    snprintf(buf, sizeof(buf), "%-3d ", errtot); 
    draw_label_val(x+2, ry, "Loi", buf, errtot ? C_RED : C_GREEN);
    ry += 9;

    snprintf(buf, sizeof(buf), "%d/%d  ", online, ui_nodeCount);
    draw_label_val(x+2, ry, "Online", buf, C_GREEN);
    ry += 9;

    snprintf(buf, sizeof(buf), "%-2d   ", ui_nodeCount);
    draw_label_val(x+2, ry, "Nodes:", buf, C_BLUE_LT);
    ry += 9;

    extern DHT11_InitTypeDef dht; 
    snprintf(buf, sizeof(buf), "%-5.1fC", dht.Temperature);
    draw_label_val(x+2, ry, "Nhiet", buf, C_GREEN);
    ry += 9;

    snprintf(buf, sizeof(buf), "%-5.1f%%", dht.Humidity);
    draw_label_val(x+2, ry, "DoAm", buf, C_GREEN);
}

/* ============================================================
 * OVERVIEW – NODE CARD 
 * ============================================================ */
#define CARD_GAP  2
#define CARD_H    45   

static void render_node_card(uint16_t cx, uint16_t cy, uint16_t cw,
                             ZbNode_t *n, uint8_t idx, uint8_t selected)
{
    uint8_t  errc  = node_err_count(n, idx);
    uint16_t bd    = selected ? C_CYAN : (errc ? C_RED : C_PANEL_BD);

    TFT_DrawRect(cx, cy, cw, CARD_H, bd);

    char buf[24];
    snprintf(buf, sizeof(buf), "%sN%d:0x%04X", selected ? ">" : " ", idx + 1, n->addr);
    TFT_DrawString(cx+2, cy+2, buf, C_CYAN, C_PANEL_BG, 1);

    const char *stat_str;
    uint16_t    stat_col;
    uint16_t    stat_bg;
    if (!n->online)          { stat_str="OFF"; stat_col=0xAA00; stat_bg=0x4220; }
    else if (errc)           { stat_str="ERR"; stat_col=C_RED;  stat_bg=C_DARK_RED; }
    else                     { stat_str="OK "; stat_col=C_GREEN;stat_bg=C_DARK_GRN; }

    TFT_FillRect(cx+cw-22, cy+1, 20, 10, stat_bg);
    TFT_DrawString(cx+cw-20, cy+2, stat_str, stat_col, stat_bg, 1);

    draw_hsep(cx+1, cy+12, cw-2);

    if (!n->online)
    {
        TFT_DrawString(cx+2, cy+16, "OFFLINE      ", C_RED, C_PANEL_BG, 1);
        snprintf(buf, sizeof(buf), "Addr:0x%04X  ", n->addr);
        TFT_DrawString(cx+2, cy+28, buf, C_GRAY, C_PANEL_BG, 1);
        return;
    }

    uint16_t col1x = cx + 2, col2x = cx + cw/2 + 1, ry = cy + 15;
    
    TFT_DrawString(col1x, ry, "NHIET", C_GRAY, C_PANEL_BG, 1);
    snprintf(buf, sizeof(buf), "%-5.1fC", n->temperature); // Chuyển sang hiển thị số thực float .1f
    TFT_DrawString(col1x+30, ry, buf, val_color_f(n->temperature, thresh_temp[idx]), C_PANEL_BG, 1);

    TFT_DrawString(col2x, ry, "DONG", C_GRAY, C_PANEL_BG, 1);
    snprintf(buf, sizeof(buf), "%-5.2fA", n->voltage_ep2); // Chuyển sang hiển thị số thực float .2f
    TFT_DrawString(col2x+25, ry, buf, val_color_f(n->voltage_ep2, thresh_curr[idx]), C_PANEL_BG, 1);
    
    ry += 11;

    TFT_DrawString(col1x, ry, "LQI", C_GRAY, C_PANEL_BG, 1);
    snprintf(buf, sizeof(buf), "%-3d  ", n->lqi);
    TFT_DrawString(col1x+20, ry, buf, lqi_color(n->lqi), C_PANEL_BG, 1);

    TFT_DrawString(col2x, ry, "NGUON", C_GRAY, C_PANEL_BG, 1);
    bool source_ok = (n->voltage_ep3 >= 3.2f);
    TFT_DrawString(col2x+32, ry, source_ok ? "OK  " : "YEU ", source_ok ? C_GREEN : C_RED, C_PANEL_BG, 1);
}

/* ============================================================
 * OVERVIEW RENDER
 * ============================================================ */
static void render_ov(void)
{
    render_ov_left();
    if (ui_nodeCount == 0) return;

    uint8_t  cols   = (ui_nodeCount <= 1) ? 1 : 2;
    uint16_t card_w = (OV_GRID_W - CARD_GAP * (cols - 1)) / cols;

    for (uint8_t i = 0; i < ui_nodeCount; i++)
    {
        uint8_t  col  = i % cols;
        uint8_t  row  = i / cols;
        uint16_t cx   = OV_GRID_X + col * (card_w + CARD_GAP);
        uint16_t cy   = BODY_Y + 2 + row * (CARD_H + CARD_GAP);

        render_node_card(cx, cy, card_w, ui_nodes[i], i, i == ui_selIdx);
    }
}

/* ============================================================
 * DETAIL VIEW – SENSOR PANEL
 * ============================================================ */
#define DT_PANEL_COUNT  2
static const char *DT_NAME[DT_PANEL_COUNT] = {"NHIET DO", "DONG DIEN"};
static const char *DT_UNIT[DT_PANEL_COUNT] = {"C", "A"};

static void render_dt_panel(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                            uint8_t pi, ZbNode_t *n, uint8_t selected)
{
    fill_box(x, y, w, h, C_PANEL_BG, selected ? C_CYAN : C_PANEL_BD);

    char lbuf[32];
    snprintf(lbuf, sizeof(lbuf), "%s%s", selected ? "> " : "  ", DT_NAME[pi]);
    TFT_DrawString(x+2, y+2, lbuf, selected ? 0x8CFF : C_GRAY, C_PANEL_BG, 1);

    char vbuf[16], sbuf[32];
    uint16_t vcol;
    uint8_t idx = ui_selIdx; 

    if (pi == 0) {
        float t_val = n->temperature;
        snprintf(vbuf, sizeof(vbuf), "%.1f", t_val); // Màn hình chi tiết in float đầy đủ
        vcol = val_color_f(t_val, thresh_temp[idx]);
        
        // Tính toán % chính xác bằng toán tử float rồi ép kiểu nguyên hiển thị
        int32_t pct = (thresh_temp[idx] != 0) ? (int32_t)((t_val * 100.0f / (float)thresh_temp[idx]) + 0.5f) : 0;
        snprintf(sbuf, sizeof(sbuf), "Nguong:%dC %d%%      ", thresh_temp[idx], (int)pct);
    } else {
        float c_val = n->voltage_ep2;
        snprintf(vbuf, sizeof(vbuf), "%.2f", c_val); // Màn hình chi tiết in float đầy đủ
        vcol = val_color_f(c_val, thresh_curr[idx]);
        
        // Tính toán % chính xác bằng toán tử float rồi ép kiểu nguyên hiển thị
        int32_t pct = (thresh_curr[idx] != 0) ? (int32_t)((c_val * 100.0f / (float)thresh_curr[idx]) + 0.5f) : 0;
        snprintf(sbuf, sizeof(sbuf), "Nguong:%dA %d%%      ", thresh_curr[idx], (int)pct);
    }

    TFT_DrawString(x+4, y+12, vbuf, vcol, C_PANEL_BG, 2);
    
    // Tự động căn khoảng cách chữ C, A theo chiều dài thực tế của chuỗi float (chống chồng đè)
    uint16_t unit_x = x + 4 + (uint16_t)(strlen(vbuf) * 12) + 2;
    TFT_DrawString(unit_x, y+16, DT_UNIT[pi], C_GRAY, C_PANEL_BG, 1);

    TFT_DrawString(x+2, y+h-9, sbuf, C_GRAY, C_PANEL_BG, 1);
}

static void render_dt_side(ZbNode_t *n)
{
    uint16_t x  = DT_SIDE_X;
    uint16_t y  = BODY_Y + 16;   
    uint16_t w  = DT_SIDE_W;
    uint16_t h1 = 60;            
    uint16_t h2 = BODY_H - 16 - h1 - CARD_GAP;  

    fill_box(x, y, w, h1, C_PANEL_BG, C_PANEL_BD);
    TFT_DrawString(x+2, y+2, "KET NOI", 0x1155, C_PANEL_BG, 1);
    draw_hsep(x+1, y+10, w-2);

    char buf[20];
    uint16_t ry = y + 13;

    snprintf(buf, sizeof(buf), "0x%04X", n->addr);
    draw_label_val(x+2, ry, "Addr", buf, C_CYAN);      ry += 9;
    draw_label_val(x+2, ry, "ZB", n->online ? "OK " : "OFF", n->online ? C_GREEN : C_RED); ry += 9;
    snprintf(buf, sizeof(buf), "%-3d", n->lqi);
    draw_label_val(x+2, ry, "LQI", buf, lqi_color(n->lqi)); ry += 9;

    const char *chat_str = n->lqi >= 200 ? "TOT " :
                           n->lqi >= 100 ? "TB  " :
                           n->lqi >=  50 ? "YEU " : "KEM ";
    draw_label_val(x+2, ry, "Chat", chat_str, lqi_color(n->lqi)); ry += 9;

    bool source_ok = (n->voltage_ep3 >= 3.2f);
    draw_label_val(x+2, ry, "Nguon", source_ok ? "OK  " : "YEU ", source_ok ? C_GREEN : C_RED);

    uint16_t wy = y + h1 + CARD_GAP;
    fill_box(x, wy, w, h2, C_PANEL_BG, C_PANEL_BD);
    TFT_DrawString(x+2, wy+2, "CANH BAO", 0x1155, C_PANEL_BG, 1);
    draw_hsep(x+1, wy+10, w-2);
    TFT_DrawString(x+2, wy+13, "Chua co log", C_GRAY, C_PANEL_BG, 1);
}

static void render_dt(void)
{
    if (ui_nodeCount == 0) return;
    ZbNode_t *n = ui_nodes[ui_selIdx];

    uint16_t dhdr_y = BODY_Y;
    TFT_FillRect(0, dhdr_y, TFT_WIDTH, 15, C_HDR_BG);
    TFT_DrawHLine(0, dhdr_y+14, TFT_WIDTH, C_HDR_LINE);

    /* BACK button */
    TFT_FillRect(2, dhdr_y+2, 22, 10, 0x0008);
    TFT_DrawRect(2, dhdr_y+2, 22, 10, 0x0034);
    TFT_DrawString(3, dhdr_y+3, "BACK", 0x88AF, C_HDR_BG, 1);

    char abuf[12];
    snprintf(abuf, sizeof(abuf), "0x%04X", n->addr);
    TFT_DrawString(28, dhdr_y+3, abuf, C_CYAN, C_HDR_BG, 1);

    /* Node tabs */
    uint16_t tx = 80;
    for (uint8_t i = 0; i < ui_nodeCount; i++)
    {
        uint16_t tb_col = (i == ui_selIdx) ? C_CYAN : C_GRAY;
        uint16_t tb_bd  = (i == ui_selIdx) ? C_CYAN : C_PANEL_BD;
        char tbuf[4]; snprintf(tbuf, sizeof(tbuf), "N%d", i+1);
        TFT_FillRect(tx, dhdr_y+2, 16, 10, C_HDR_BG);
        TFT_DrawRect(tx, dhdr_y+2, 16, 10, tb_bd);
        TFT_DrawString(tx+2, dhdr_y+3, tbuf, tb_col, C_HDR_BG, 1);
        tx += 18;
    }

    bool source_ok = (n->voltage_ep3 >= 3.2f);
    char nbuf[12]; 
    snprintf(nbuf, sizeof(nbuf), "NG:%s", source_ok ? "OK " : "YEU");
    uint16_t bx = DT_SIDE_X;
    TFT_FillRect(bx-10, dhdr_y+2, 38, 10, source_ok ? 0x0540 : 0x2800);
    TFT_DrawString(bx-8, dhdr_y+3, nbuf, source_ok ? C_GREEN : C_RED, source_ok ? 0x0540 : 0x2800, 1);

    char zbuf[10];
    snprintf(zbuf, sizeof(zbuf), "ZB:%s", n->online ? "OK " : "OFF");
    uint16_t zx = bx + 32;
    TFT_FillRect(zx, dhdr_y+2, 34, 10, n->online ? 0x0540 : 0x2800);
    TFT_DrawString(zx+1, dhdr_y+3, zbuf, n->online ? C_GREEN : C_RED, n->online ? 0x0540 : 0x2800, 1);

    uint16_t py     = BODY_Y + 16;
    uint16_t pw     = DT_SIDE_X - 4;
    uint16_t ph     = (BODY_H - 16 - 1 * CARD_GAP) / DT_PANEL_COUNT;

    for (uint8_t pi = 0; pi < DT_PANEL_COUNT; pi++) {
        render_dt_panel(2, py, pw, ph, pi, n, pi == ui_dtPanel);
        py += ph + CARD_GAP;
    }

    render_dt_side(n);
}

/* ============================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================ */
void UI_Init(ZbNode_t **nodes, uint8_t count)
{
    ui_nodeCount = count > MAX_NODES ? MAX_NODES : count;
    for (uint8_t i = 0; i < ui_nodeCount; i++) ui_nodes[i] = nodes[i];
    ui_selIdx    = 0;
    ui_view      = UI_VIEW_OV;
    ui_view_prev = 255; 
    ui_dtPanel   = 0;
}

void UI_Render(void)
{
    uint8_t total_err = 0;
    global_hw_error = 0;
    
    for (uint8_t i = 0; i < ui_nodeCount; i++) {
        ZbNode_t *n = ui_nodes[i];
        total_err += node_err_count(n, i);
        if (n->online && (n->temperature > (float)thresh_temp[i] || n->voltage_ep2 > (float)thresh_curr[i])) {
            global_hw_error = 1; 
        }
    }

    if (global_hw_error) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);   
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); 
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); 
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); 
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);   
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); 
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0); 
    }

    /* Quét sạch sành sanh TOÀN MÀN HÌNH (0,0 -> 320,240) khi đổi trang */
    if (ui_view != ui_view_prev) {
        TFT_FillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, C_BG);
        ui_view_prev = ui_view;
    }

    render_header();

    if (ui_view == UI_VIEW_OV) render_ov();
    else render_dt();

    if (total_err == 0) {
        render_statusbar(0, NULL);
    } else {
        for (uint8_t i = 0; i < ui_nodeCount; i++) {
            ZbNode_t *n = ui_nodes[i];
            char errbuf[40];
            if (!n->online) {
                snprintf(errbuf, sizeof(errbuf), "N%d: NODE OFFLINE", i+1);
                render_statusbar(1, errbuf); return;
            }
            if (n->lqi < 50) {
                snprintf(errbuf, sizeof(errbuf), "N%d: ZB LQI THAP (%d)", i+1, n->lqi);
                render_statusbar(1, errbuf); return;
            }
            if (n->temperature > (float)thresh_temp[i]) {
                snprintf(errbuf, sizeof(errbuf), "N%d: NHIET CAO %.1fC", i+1, n->temperature); // Xuất float ra status bar
                render_statusbar(1, errbuf); return;
            }
            if (n->voltage_ep2 > (float)thresh_curr[i]) {
                snprintf(errbuf, sizeof(errbuf), "N%d: DONG CAO %.2fA", i+1, n->voltage_ep2); // Xuất float ra status bar
                render_statusbar(1, errbuf); return;
            }
        }
        render_statusbar(1, "LOI HE THONG");
    }
}

void UI_Button(uint8_t btn)
{
    if (ui_nodeCount == 0) return;

    if (ui_view == UI_VIEW_OV) {
        switch (btn) {
            case 1: ui_selIdx = (ui_selIdx == 0) ? ui_nodeCount - 1 : ui_selIdx - 1; break;
            case 2: ui_selIdx = (ui_selIdx + 1) % ui_nodeCount; break;
            case 3: 
                ui_view    = UI_VIEW_DT;
                ui_dtPanel = 0;
                break;
        }
    } else {
        switch (btn) {
            case 0: ui_view = UI_VIEW_OV; break;
            case 1:
                ui_selIdx  = (ui_selIdx == 0) ? ui_nodeCount - 1 : ui_selIdx - 1;
                ui_dtPanel = 0;
                break;
            case 2:
                ui_selIdx  = (ui_selIdx + 1) % ui_nodeCount;
                ui_dtPanel = 0;
                break;
            case 3:
                ui_dtPanel = (ui_dtPanel + 1) % DT_PANEL_COUNT;
                break;
        }
    }
		ui_view_prev = 255;
    UI_Render();
}

void UI_Update(void) {
    UI_Render();
}

void UI_SetNodeThreshold(uint16_t addr, int16_t temp_th, int16_t curr_th) {
    int8_t idx = find_node_index(addr);
    if (idx != -1) {
        thresh_temp[idx] = temp_th; 
        thresh_curr[idx] = curr_th;  
        UI_Render();                 
    }
}

void UI_Alarm_Tick(void) {
    if (!global_hw_error) return; 
    static uint32_t last_blink = 0;
    uint32_t now = HAL_GetTick();
    uint32_t diff = now - last_blink;
    if (diff >= 2000) { last_blink = now; diff = 0; }
    if (diff < 200) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1); 
    } else {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);   
    }
}