/**
 * @file market.c
 * @brief Real currency & market engine — fluctuating rates, commodities, companies
 */
#include "../../include/core/market.h"
#include "../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const struct { const char *iso, *name, *symbol; float base_rate, volatility; } s_currency_defs[] = {
  {"USD","US Dollar","$",1.000f,0.008f},{"CAD","Canadian Dollar","$",1.360f,0.008f},
  {"MXN","Mexican Peso","$",16.70f,0.014f},{"BRL","Brazilian Real","R$",4.970f,0.018f},
  {"ARS","Argentine Peso","$",890.0f,0.040f},{"CLP","Chilean Peso","$",920.0f,0.022f},
  {"COP","Colombian Peso","$",3890.0f,0.020f},{"PEN","Peruvian Sol","S/",3.720f,0.012f},
  {"EUR","Euro","€",0.920f,0.007f},{"GBP","British Pound","£",0.790f,0.009f},
  {"CHF","Swiss Franc","Fr",0.890f,0.006f},{"SEK","Swedish Krona","kr",10.45f,0.010f},
  {"NOK","Norwegian Krone","kr",10.60f,0.011f},{"DKK","Danish Krone","kr",6.870f,0.007f},
  {"PLN","Polish Zloty","zł",3.950f,0.012f},{"CZK","Czech Koruna","Kč",22.80f,0.010f},
  {"HUF","Hungarian Forint","Ft",360.0f,0.014f},{"RON","Romanian Leu","lei",4.580f,0.012f},
  {"NGN","Nigerian Naira","₦",1540.0f,0.035f},{"KES","Kenyan Shilling","KSh",129.5f,0.015f},
  {"ZAR","South African Rand","R",18.60f,0.022f},{"EGP","Egyptian Pound","£E",47.80f,0.028f},
  {"GHS","Ghanaian Cedi","GH₵",12.80f,0.030f},{"MAD","Moroccan Dirham","DH",10.10f,0.012f},
  {"ETB","Ethiopian Birr","Br",57.20f,0.025f},{"XOF","CFA Franc BCEAO","CFA",610.0f,0.014f},
  {"XAF","CFA Franc BEAC","CFA",610.0f,0.014f},{"UGX","Ugandan Shilling","USh",3800.0f,0.020f},
  {"TZS","Tanzanian Shilling","TSh",2550.0f,0.018f},{"RWF","Rwandan Franc","RFr",1300.0f,0.020f},
  {"SAR","Saudi Riyal","﷼",3.750f,0.004f},{"AED","UAE Dirham","د.إ",3.670f,0.004f},
  {"QAR","Qatari Riyal","﷼",3.640f,0.004f},{"KWD","Kuwaiti Dinar","KD",0.307f,0.003f},
  {"ILS","Israeli Shekel","₪",3.680f,0.012f},{"JPY","Japanese Yen","¥",149.50f,0.010f},
  {"CNY","Chinese Yuan","¥",7.240f,0.006f},{"INR","Indian Rupee","₹",83.10f,0.012f},
  {"KRW","South Korean Won","₩",1330.0f,0.011f},{"SGD","Singapore Dollar","$",1.340f,0.006f},
  {"HKD","Hong Kong Dollar","HK$",7.820f,0.005f},{"MYR","Malaysian Ringgit","RM",4.680f,0.012f},
  {"IDR","Indonesian Rupiah","Rp",15600.0f,0.020f},{"THB","Thai Baht","฿",36.20f,0.012f},
  {"VND","Vietnamese Dong","₫",24800.0f,0.018f},{"PHP","Philippine Peso","₱",56.20f,0.014f},
  {"PKR","Pakistani Rupee","Rs",278.0f,0.025f},{"BDT","Bangladeshi Taka","৳",110.0f,0.020f},
  {"AUD","Australian Dollar","$",1.520f,0.010f},{"NZD","New Zealand Dollar","$",1.630f,0.012f},
  {"TRY","Turkish Lira","₺",32.10f,0.030f},{"RUB","Russian Ruble","₽",92.30f,0.025f},
};

static const struct { const char *name, *unit; float price, volatility; } s_commodity_defs[] = {
  {"Gold","oz",2340.0f,0.012f},{"Crude Oil","bbl",78.5f,0.025f},
  {"Wheat","tonne",220.0f,0.018f},{"Copper","tonne",8500.0f,0.015f},
  {"Coffee","tonne",3200.0f,0.022f},{"Natural Gas","MMBtu",3.20f,0.030f},
  {"Silver","oz",27.5f,0.016f},{"Corn","tonne",180.0f,0.020f},
  {"Iron Ore","tonne",110.0f,0.014f},{"Cotton","tonne",2100.0f,0.018f},
  {"Sugar","tonne",550.0f,0.022f},{"Rice","tonne",420.0f,0.015f},
};

civ_market_engine_t *civ_market_create(void) {
  civ_market_engine_t *m = (civ_market_engine_t *)malloc(sizeof(civ_market_engine_t));
  if (!m) return NULL; memset(m, 0, sizeof(*m));
  int nc = sizeof(s_currency_defs)/sizeof(s_currency_defs[0]);
  m->currency_count = nc < CIV_CURRENCY_MAX ? nc : CIV_CURRENCY_MAX;
  for (int i = 0; i < m->currency_count; i++) {
    civ_market_currency_t *c = &m->currencies[i];
    strncpy(c->iso, s_currency_defs[i].iso, 3);
    strncpy(c->name, s_currency_defs[i].name, CIV_CURRENCY_NAME-1);
    strncpy(c->symbol, s_currency_defs[i].symbol, 3);
    c->base_rate = s_currency_defs[i].base_rate;
    c->current_rate = s_currency_defs[i].base_rate;
    c->volatility = s_currency_defs[i].volatility;
    c->inflation = 0.02f;
  }
  int ncom = sizeof(s_commodity_defs)/sizeof(s_commodity_defs[0]);
  m->commodity_count = ncom < 12 ? ncom : 12;
  for (int i = 0; i < m->commodity_count; i++) {
    civ_commodity_t *co = &m->commodities[i];
    strncpy(co->name, s_commodity_defs[i].name, 31);
    strncpy(co->unit, s_commodity_defs[i].unit, 7);
    co->price_per_unit = s_commodity_defs[i].price;
    co->volatility = s_commodity_defs[i].volatility;
    co->supply_index = 0.5f; co->demand_index = 0.5f;
  }
  return m;
}

void civ_market_destroy(civ_market_engine_t *m) { free(m); }

civ_market_currency_t *civ_market_add_currency(civ_market_engine_t *m,
    const char *iso, const char *name, const char *symbol, float rate) {
  if (!m || m->currency_count >= CIV_CURRENCY_MAX) return NULL;
  civ_market_currency_t *c = &m->currencies[m->currency_count++];
  memset(c, 0, sizeof(*c));
  strncpy(c->iso, iso, 3); strncpy(c->name, name, CIV_CURRENCY_NAME-1);
  strncpy(c->symbol, symbol, 3);
  c->base_rate = rate; c->current_rate = rate;
  c->volatility = 0.015f; c->inflation = 0.02f;
  return c;
}

void civ_market_update(civ_market_engine_t *m) {
  if (!m) return;
  for (int i = 0; i < m->currency_count; i++) {
    civ_market_currency_t *c = &m->currencies[i];
    float swing = ((float)(rand()%100)/100.0f - 0.5f) * c->volatility * 2.0f;
    c->current_rate *= (1.0f + swing);
    c->current_rate *= (1.0f + c->inflation * 0.001f);
    if (c->current_rate < c->base_rate * 0.5f) c->current_rate = c->base_rate * 0.5f;
    if (c->current_rate > c->base_rate * 3.0f) c->current_rate = c->base_rate * 3.0f;
  }
  for (int i = 0; i < m->commodity_count; i++) {
    civ_commodity_t *co = &m->commodities[i];
    float ss = ((float)(rand()%100)/100.0f - 0.5f) * 0.05f;
    float ds = ((float)(rand()%100)/100.0f - 0.5f) * 0.05f;
    co->supply_index += ss; co->demand_index += ds;
    if (co->supply_index < 0.1f) co->supply_index = 0.1f;
    if (co->supply_index > 0.9f) co->supply_index = 0.9f;
    if (co->demand_index < 0.1f) co->demand_index = 0.1f;
    if (co->demand_index > 0.9f) co->demand_index = 0.9f;
    float pm = (co->demand_index - co->supply_index) * co->volatility;
    co->price_per_unit *= (1.0f + pm);
  }
}

civ_market_currency_t *civ_market_get_currency(civ_market_engine_t *m, const char *iso) {
  if (!m || !iso) return NULL;
  for (int i = 0; i < m->currency_count; i++)
    if (strcmp(m->currencies[i].iso, iso) == 0) return &m->currencies[i];
  return NULL;
}

float civ_market_exchange(civ_market_engine_t *m, const char *from, const char *to, float amt) {
  if (!m || !from || !to) return amt;
  civ_market_currency_t *fc = civ_market_get_currency(m, from);
  civ_market_currency_t *tc = civ_market_get_currency(m, to);
  if (!fc || !tc) return amt;
  return (amt / fc->current_rate) * tc->current_rate;
}

void civ_wallet_init(civ_wallet_t *w) { memset(w, 0, sizeof(*w)); }

void civ_wallet_add(civ_wallet_t *w, const char *iso, float amount) {
  if (!w || !iso) return;
  for (int i = 0; i < w->count; i++)
    if (strcmp(w->slots[i].currency_iso, iso) == 0) { w->slots[i].balance += amount; return; }
  if (w->count >= CIV_WALLET_SLOTS) return;
  strncpy(w->slots[w->count].currency_iso, iso, 3);
  w->slots[w->count].balance = amount; w->count++;
}

float civ_wallet_balance(civ_wallet_t *w, const char *iso) {
  if (!w || !iso) return 0;
  for (int i = 0; i < w->count; i++)
    if (strcmp(w->slots[i].currency_iso, iso) == 0) return w->slots[i].balance;
  return 0;
}

void civ_wallet_remove(civ_wallet_t *w, const char *iso) {
  if (!w || !iso) return;
  for (int i = 0; i < w->count; i++)
    if (strcmp(w->slots[i].currency_iso, iso) == 0) {
      for (int j = i; j < w->count-1; j++) w->slots[j] = w->slots[j+1];
      w->count--; return;
    }
}

bool civ_wallet_exchange(civ_wallet_t *w, civ_market_engine_t *m,
                         const char *from, const char *to, float amount) {
  if (!w || !m || amount <= 0) return false;
  float bal = civ_wallet_balance(w, from);
  if (bal < amount) return false;
  float conv = civ_market_exchange(m, from, to, amount);
  civ_wallet_remove(w, from);
  civ_wallet_add(w, from, bal - amount);
  civ_wallet_add(w, to, conv);
  return true;
}

float civ_wallet_total(civ_wallet_t *w, civ_market_engine_t *m) {
  if (!w || !m) return 0; float total = 0;
  for (int i = 0; i < w->count; i++) {
    civ_market_currency_t *c = civ_market_get_currency(m, w->slots[i].currency_iso);
    if (c) total += w->slots[i].balance / c->current_rate;
  }
  return total;
}

civ_commodity_t *civ_market_get_commodity(civ_market_engine_t *m, const char *name) {
  if (!m || !name) return NULL;
  for (int i = 0; i < m->commodity_count; i++)
    if (strcmp(m->commodities[i].name, name) == 0) return &m->commodities[i];
  return NULL;
}

float civ_market_dynamic_price(civ_market_engine_t *m, float base_price,
                               const char *currency_iso, float cost_of_living) {
  civ_market_currency_t *c = civ_market_get_currency(m, currency_iso);
  if (!c) return base_price * cost_of_living;
  return base_price * cost_of_living * c->current_rate;
}

void civ_market_generate_companies(civ_market_engine_t *m, const char *nation_id) {
  if (!m || m->company_count >= CIV_COMPANY_MAX) return;
  const char *industries[] = {"Agriculture","Technology","Finance","Manufacturing",
      "Retail","Energy","Healthcare","Construction","Transport","Media"};
  const char *city_sfx[] = {"Capital","Metro","Holdings","Group","Industries",
      "Corp","Exchange","Enterprises","Ventures","Partners"};
  for (int i = 0; i < 8 && m->company_count < CIV_COMPANY_MAX; i++) {
    civ_company_t *co = &m->companies[m->company_count++];
    int si = ((int)(unsigned char)nation_id[0] * 31 +
              (int)(unsigned char)nation_id[1] * 7 + i * 13) % 10;
    if (si < 0) si = -si;
    snprintf(co->name, sizeof(co->name), "%s %s %s", nation_id, industries[i%10], city_sfx[si]);
    snprintf(co->industry, sizeof(co->industry), "%s", industries[i%10]);
    snprintf(co->nation_id, sizeof(co->nation_id), "%s", nation_id);
    co->employees = 50 + (rand() % 5000);
    co->revenue = co->employees * (500.0f + (rand() % 5000));
    co->stock_price = (i%3==0) ? (10.0f + (rand()%200)) : 0.0f;
    co->is_public = (i%3==0);
    co->growth_rate = ((rand()%2000)/100.0f - 5.0f)/100.0f;
  }
}

civ_company_t *civ_market_get_company(civ_market_engine_t *m, int idx) {
  if (!m || idx<0||idx>=m->company_count) return NULL;
  return &m->companies[idx];
}

float civ_market_cost_of_living(civ_market_engine_t *m, const char *iso) {
  civ_market_currency_t *c = civ_market_get_currency(m, iso);
  return c ? 0.8f + c->inflation*10.0f : 1.0f;
}
