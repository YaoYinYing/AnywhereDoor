#ifndef ANYWHEREDOOR_TUI_H
#define ANYWHEREDOOR_TUI_H

/* Launch the interactive ncurses TUI.
   Returns: 0 if user quit without selecting (no shell output),
            1 if user selected a proxy (export statements printed to stdout),
            2 if user toggled off (unset statements printed to stdout). */
int tui_run(void);

#endif /* ANYWHEREDOOR_TUI_H */
