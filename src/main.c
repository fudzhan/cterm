
#include "cterm.h"

int main(int argc, char** argv) {
    CTerm term;
    GtkRcStyle* style;
    struct sigaction ignore_children;

    /* Avoid zombies when executing external programs by explicitly setting the
       handler to SIG_IGN */
    ignore_children.sa_handler = SIG_IGN;
    ignore_children.sa_flags = 0;
    sigemptyset(&ignore_children.sa_mask);
    sigaction(SIGCHLD, &ignore_children, NULL);

    /* Initialize GTK */
    gtk_init(&argc, &argv);

    /* Initialize CTerm data structure */
    term.terminal_procs = g_hash_table_new(NULL, g_int_equal);
    term.window = (GtkWindow*) gtk_window_new(GTK_WINDOW_TOPLEVEL);
    term.notebook = (GtkNotebook*) gtk_notebook_new();
    term.count = 0;

    /* Load configuration options */
    cterm_init_config_defaults(&term);
    cterm_reread_config(&term);

    /* Set title */
    gtk_window_set_title(term.window, "cterm");

    /* Optionally hide window from taskbar */
    if(getenv("CTERM_HIDE") != NULL) {
        gtk_window_set_skip_taskbar_hint(term.window, true);
        gtk_window_set_skip_pager_hint(term.window, true);
    }

    gtk_notebook_set_scrollable(term.notebook, FALSE);
    gtk_notebook_set_show_tabs(term.notebook, FALSE);
    gtk_notebook_set_show_border(term.notebook, FALSE);

    g_object_set(G_OBJECT(term.notebook), "show-border", FALSE, NULL);
    g_object_set(G_OBJECT(term.notebook), "homogeneous", TRUE, NULL);

    /* Disable all borders on notebook */
    style = gtk_rc_style_new();
    style->xthickness = 0;
    style->ythickness = 0;
    gtk_widget_modify_style(GTK_WIDGET(term.notebook), style);

    /* Connect signals */
    g_signal_connect(term.notebook, "switch-page", G_CALLBACK(cterm_ontabchange), &term);

    /* Build main window */
    gtk_container_add(GTK_CONTAINER(term.window), GTK_WIDGET(term.notebook));

    /* Confirm exit on window close.
       Event propagates to gtk_main_quit if cterm_onwindowclose returns FALSE. */
    g_signal_connect(term.window, "delete-event", G_CALLBACK(cterm_onwindowclose), &term);
    g_signal_connect(term.window, "delete-event", gtk_main_quit, NULL);

    /* Open initial tab */
    cterm_open_tab(&term);

    /* Resize Window */
    cterm_set_term_size(&term,
                        term.config.initial_width, term.config.initial_height,
                        term.config.width_unit, term.config.height_unit);

    /* Show window and enter main event loop */
    gtk_widget_show_all(GTK_WIDGET(term.window));
    gtk_main();

    return 0;
}
