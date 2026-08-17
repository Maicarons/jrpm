/** @file jrpm_watch_gui.h
 * Watch another company's building activity (cmclient "Watch company's actions" port).
 */

#ifndef JRPM_WATCH_GUI_H
#define JRPM_WATCH_GUI_H

#include "company_type.h"
#include "tile_type.h"

/** Show the watch-company window, optionally focused on a company. */
void ShowWatchWindow(CompanyID company_to_watch = CompanyID::Invalid());

/** Notify all open watch windows that @p company built at @p tile (so they can follow). */
void UpdateWatching(CompanyID company, TileIndex tile);

#endif /* JRPM_WATCH_GUI_H */
