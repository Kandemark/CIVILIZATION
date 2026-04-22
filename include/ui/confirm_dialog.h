/**
 * @file confirm_dialog.h
 * @brief Reusable confirmation popup — "Are you sure?" with Confirm/Cancel
 *
 * Uses the window manager. Any scene can call civ_confirm_show()
 * and check civ_confirm_result() to see what the user chose.
 */
#ifndef CIV_UI_CONFIRM_DIALOG_H
#define CIV_UI_CONFIRM_DIALOG_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Show a confirmation dialog. Returns immediately; check result later. */
void civ_confirm_show(const char *title, const char *message);

/* Returns: 1 = confirmed, -1 = cancelled, 0 = still open / never shown */
int  civ_confirm_result(void);

/* True while the dialog is visible */
bool civ_confirm_is_open(void);

/* Force-close the dialog */
void civ_confirm_close(void);

#ifdef __cplusplus
}
#endif
#endif
