/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "gtkconcb.h"
#include "gtkconnect.h"
#include "gtksupport.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_Connections (void)
{
  GtkWidget *Connections;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *label1;
  GtkWidget *hseparator1;
  GtkWidget *hbox1;
  GtkWidget *scrolledwindow1;
  GtkWidget *ConnectionsTreeView;
  GtkWidget *vbuttonbox1;
  GtkWidget *button1;
  GtkWidget *button2;
  GtkWidget *button3;
  GtkWidget *button4;
  GtkWidget *dialog_action_area1;
  GtkWidget *revertbutton1;
  GtkWidget *cancelbutton1;
  GtkWidget *connectbutton1;
  GtkWidget *alignment2;
  GtkWidget *hbox7;
  GtkWidget *image3;
  GtkWidget *label13;

  Connections = gtk_dialog_new ();
  gtk_widget_set_name (Connections, "Connections");
  gtk_window_set_title (GTK_WINDOW (Connections), "GtkHack");

  dialog_vbox1 = GTK_DIALOG (Connections)->vbox;
  gtk_widget_set_name (dialog_vbox1, "dialog_vbox1");
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox1, "vbox1");
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  label1 = gtk_label_new ("You may choose an existing server to connect to here, or configure a new one.");
  gtk_widget_set_name (label1, "label1");
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (vbox1), label1, FALSE, FALSE, 10);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator1, "hseparator1");
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (vbox1), hseparator1, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (hbox1, "hbox1");
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 5);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow1, "scrolledwindow1");
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow1, TRUE, TRUE, 5);
  GTK_WIDGET_SET_FLAGS (scrolledwindow1, GTK_CAN_DEFAULT);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  ConnectionsTreeView = gtk_tree_view_new ();
  gtk_widget_set_name (ConnectionsTreeView, "ConnectionsTreeView");
  gtk_widget_show (ConnectionsTreeView);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), ConnectionsTreeView);
  GTK_WIDGET_SET_FLAGS (ConnectionsTreeView, GTK_CAN_DEFAULT);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (ConnectionsTreeView), TRUE);

  vbuttonbox1 = gtk_vbutton_box_new ();
  gtk_widget_set_name (vbuttonbox1, "vbuttonbox1");
  gtk_widget_show (vbuttonbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbuttonbox1, FALSE, TRUE, 7);
  gtk_container_set_border_width (GTK_CONTAINER (vbuttonbox1), 5);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox1), GTK_BUTTONBOX_START);
  gtk_box_set_spacing (GTK_BOX (vbuttonbox1), 5);

  button1 = gtk_button_new_with_mnemonic ("_Add...");
  gtk_widget_set_name (button1, "button1");
  gtk_widget_show (button1);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), button1);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  button2 = gtk_button_new_with_mnemonic ("_Edit...");
  gtk_widget_set_name (button2, "button2");
  gtk_widget_show (button2);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), button2);
  GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);

  button3 = gtk_button_new_with_mnemonic ("_Delete");
  gtk_widget_set_name (button3, "button3");
  gtk_widget_show (button3);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), button3);
  GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);

  button4 = gtk_button_new_with_mnemonic ("_Set as default");
  gtk_widget_set_name (button4, "button4");
  gtk_widget_show (button4);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), button4);
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);

  dialog_action_area1 = GTK_DIALOG (Connections)->action_area;
  gtk_widget_set_name (dialog_action_area1, "dialog_action_area1");
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  revertbutton1 = gtk_button_new_from_stock ("gtk-revert-to-saved");
  gtk_widget_set_name (revertbutton1, "revertbutton1");
  gtk_widget_show (revertbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (Connections), revertbutton1, 0);
  GTK_WIDGET_SET_FLAGS (revertbutton1, GTK_CAN_DEFAULT);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_set_name (cancelbutton1, "cancelbutton1");
  gtk_widget_show (cancelbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (Connections), cancelbutton1, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

  connectbutton1 = gtk_button_new ();
  gtk_widget_set_name (connectbutton1, "connectbutton1");
  gtk_widget_show (connectbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (Connections), connectbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (connectbutton1, GTK_CAN_DEFAULT);

  alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_set_name (alignment2, "alignment2");
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (connectbutton1), alignment2);

  hbox7 = gtk_hbox_new (FALSE, 2);
  gtk_widget_set_name (hbox7, "hbox7");
  gtk_widget_show (hbox7);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox7);

  image3 = gtk_image_new_from_stock ("gtk-yes", GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_name (image3, "image3");
  gtk_widget_show (image3);
  gtk_box_pack_start (GTK_BOX (hbox7), image3, FALSE, FALSE, 0);

  label13 = gtk_label_new_with_mnemonic ("C_onnect");
  gtk_widget_set_name (label13, "label13");
  gtk_widget_show (label13);
  gtk_box_pack_start (GTK_BOX (hbox7), label13, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_LEFT);

  g_signal_connect ((gpointer) button1, "clicked",
                    G_CALLBACK (GTK_add_connection),
                    NULL);
  g_signal_connect_swapped ((gpointer) button2, "clicked",
                            G_CALLBACK (GTK_edit_connection),
                            GTK_OBJECT (ConnectionsTreeView));
  g_signal_connect_swapped ((gpointer) button3, "clicked",
                            G_CALLBACK (GTK_delete_connection),
                            GTK_OBJECT (ConnectionsTreeView));
  g_signal_connect_swapped ((gpointer) button4, "clicked",
                            G_CALLBACK (GTK_set_default_connection),
                            GTK_OBJECT (ConnectionsTreeView));
  g_signal_connect_swapped ((gpointer) revertbutton1, "clicked",
                            G_CALLBACK (GTK_revert_connections),
                            GTK_OBJECT (ConnectionsTreeView));

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (Connections, Connections, "Connections");
  GLADE_HOOKUP_OBJECT_NO_REF (Connections, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (Connections, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (Connections, label1, "label1");
  GLADE_HOOKUP_OBJECT (Connections, hseparator1, "hseparator1");
  GLADE_HOOKUP_OBJECT (Connections, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (Connections, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (Connections, ConnectionsTreeView, "ConnectionsTreeView");
  GLADE_HOOKUP_OBJECT (Connections, vbuttonbox1, "vbuttonbox1");
  GLADE_HOOKUP_OBJECT (Connections, button1, "button1");
  GLADE_HOOKUP_OBJECT (Connections, button2, "button2");
  GLADE_HOOKUP_OBJECT (Connections, button3, "button3");
  GLADE_HOOKUP_OBJECT (Connections, button4, "button4");
  GLADE_HOOKUP_OBJECT_NO_REF (Connections, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (Connections, revertbutton1, "revertbutton1");
  GLADE_HOOKUP_OBJECT (Connections, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (Connections, connectbutton1, "connectbutton1");
  GLADE_HOOKUP_OBJECT (Connections, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (Connections, hbox7, "hbox7");
  GLADE_HOOKUP_OBJECT (Connections, image3, "image3");
  GLADE_HOOKUP_OBJECT (Connections, label13, "label13");

  gtk_widget_grab_focus (connectbutton1);
  gtk_widget_grab_default (connectbutton1);
  return Connections;
}

GtkWidget*
create_AddConnection (void)
{
  GtkWidget *AddConnection;
  GtkWidget *dialog_vbox2;
  GtkWidget *vbox2;
  GtkWidget *label9;
  GtkWidget *hbox5;
  GtkWidget *label10;
  GtkWidget *ConnectionName;
  GtkWidget *hseparator2;
  GtkWidget *label3;
  GtkWidget *hbox3;
  GtkWidget *label4;
  GtkWidget *combo1;
  GList *combo1_items = NULL;
  GtkWidget *ServerType;
  GtkWidget *hseparator3;
  GtkWidget *label14;
  GtkWidget *DisableAsync;
  GtkWidget *dialog_action_area2;
  GtkWidget *cancelbutton2;
  GtkWidget *okbutton2;

  AddConnection = gtk_dialog_new ();
  gtk_widget_set_name (AddConnection, "AddConnection");
  gtk_window_set_title (GTK_WINDOW (AddConnection), "Add Connection");
  gtk_window_set_modal (GTK_WINDOW (AddConnection), TRUE);

  dialog_vbox2 = GTK_DIALOG (AddConnection)->vbox;
  gtk_widget_set_name (dialog_vbox2, "dialog_vbox2");
  gtk_widget_show (dialog_vbox2);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox2, "vbox2");
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (dialog_vbox2), vbox2, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  label9 = gtk_label_new ("Please select a name for this server connection.");
  gtk_widget_set_name (label9, "label9");
  gtk_widget_show (label9);
  gtk_box_pack_start (GTK_BOX (vbox2), label9, FALSE, FALSE, 10);
  gtk_label_set_justify (GTK_LABEL (label9), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

  hbox5 = gtk_hbox_new (FALSE, 5);
  gtk_widget_set_name (hbox5, "hbox5");
  gtk_widget_show (hbox5);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox5, TRUE, TRUE, 0);

  label10 = gtk_label_new_with_mnemonic ("_Connection name:");
  gtk_widget_set_name (label10, "label10");
  gtk_widget_show (label10);
  gtk_box_pack_start (GTK_BOX (hbox5), label10, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label10), GTK_JUSTIFY_LEFT);

  ConnectionName = gtk_entry_new ();
  gtk_widget_set_name (ConnectionName, "ConnectionName");
  gtk_widget_show (ConnectionName);
  gtk_box_pack_start (GTK_BOX (hbox5), ConnectionName, TRUE, TRUE, 0);

  hseparator2 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator2, "hseparator2");
  gtk_widget_show (hseparator2);
  gtk_box_pack_start (GTK_BOX (vbox2), hseparator2, TRUE, TRUE, 5);

  label3 = gtk_label_new ("Select the server type, you want to connect to");
  gtk_widget_set_name (label3, "label3");
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (vbox2), label3, FALSE, FALSE, 10);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label3), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  hbox3 = gtk_hbox_new (FALSE, 5);
  gtk_widget_set_name (hbox3, "hbox3");
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox3, FALSE, FALSE, 0);

  label4 = gtk_label_new_with_mnemonic ("_Server type:");
  gtk_widget_set_name (label4, "label4");
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (hbox3), label4, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);

  combo1 = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (combo1)->popwin),
                     "GladeParentKey", combo1);
  gtk_widget_set_name (combo1, "combo1");
  gtk_widget_show (combo1);
  gtk_box_pack_start (GTK_BOX (hbox3), combo1, TRUE, TRUE, 0);
  gtk_combo_set_value_in_list (GTK_COMBO (combo1), TRUE, FALSE);
  combo1_items = g_list_append (combo1_items, (gpointer) "Local executable");
  combo1_items = g_list_append (combo1_items, (gpointer) "Remote machine");
  gtk_combo_set_popdown_strings (GTK_COMBO (combo1), combo1_items);
  g_list_free (combo1_items);

  ServerType = GTK_COMBO (combo1)->entry;
  gtk_widget_set_name (ServerType, "ServerType");
  gtk_widget_show (ServerType);
  gtk_editable_set_editable (GTK_EDITABLE (ServerType), FALSE);
  gtk_entry_set_activates_default (GTK_ENTRY (ServerType), TRUE);

  hseparator3 = gtk_hseparator_new ();
  gtk_widget_set_name (hseparator3, "hseparator3");
  gtk_widget_show (hseparator3);
  gtk_box_pack_start (GTK_BOX (vbox2), hseparator3, TRUE, TRUE, 5);

  label14 = gtk_label_new ("Troubleshooting");
  gtk_widget_set_name (label14, "label14");
  gtk_widget_show (label14);
  gtk_box_pack_start (GTK_BOX (vbox2), label14, FALSE, FALSE, 10);
  gtk_label_set_justify (GTK_LABEL (label14), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label14), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label14), 0, 0.5);

  DisableAsync = gtk_check_button_new_with_mnemonic ("Disable _asynchronous support (will increase latency)");
  gtk_widget_set_name (DisableAsync, "DisableAsync");
  gtk_widget_show (DisableAsync);
  gtk_box_pack_start (GTK_BOX (vbox2), DisableAsync, FALSE, FALSE, 0);

  dialog_action_area2 = GTK_DIALOG (AddConnection)->action_area;
  gtk_widget_set_name (dialog_action_area2, "dialog_action_area2");
  gtk_widget_show (dialog_action_area2);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area2), GTK_BUTTONBOX_END);

  cancelbutton2 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_set_name (cancelbutton2, "cancelbutton2");
  gtk_widget_show (cancelbutton2);
  gtk_dialog_add_action_widget (GTK_DIALOG (AddConnection), cancelbutton2, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton2, GTK_CAN_DEFAULT);

  okbutton2 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_set_name (okbutton2, "okbutton2");
  gtk_widget_show (okbutton2);
  gtk_dialog_add_action_widget (GTK_DIALOG (AddConnection), okbutton2, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton2, GTK_CAN_DEFAULT);

  g_signal_connect_swapped ((gpointer) cancelbutton2, "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (AddConnection));
  g_signal_connect_swapped ((gpointer) okbutton2, "clicked",
                            G_CALLBACK (GTK_create_connection),
                            GTK_OBJECT (AddConnection));

  gtk_label_set_mnemonic_widget (GTK_LABEL (label10), ConnectionName);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label4), ServerType);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (AddConnection, AddConnection, "AddConnection");
  GLADE_HOOKUP_OBJECT_NO_REF (AddConnection, dialog_vbox2, "dialog_vbox2");
  GLADE_HOOKUP_OBJECT (AddConnection, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (AddConnection, label9, "label9");
  GLADE_HOOKUP_OBJECT (AddConnection, hbox5, "hbox5");
  GLADE_HOOKUP_OBJECT (AddConnection, label10, "label10");
  GLADE_HOOKUP_OBJECT (AddConnection, ConnectionName, "ConnectionName");
  GLADE_HOOKUP_OBJECT (AddConnection, hseparator2, "hseparator2");
  GLADE_HOOKUP_OBJECT (AddConnection, label3, "label3");
  GLADE_HOOKUP_OBJECT (AddConnection, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (AddConnection, label4, "label4");
  GLADE_HOOKUP_OBJECT (AddConnection, combo1, "combo1");
  GLADE_HOOKUP_OBJECT (AddConnection, ServerType, "ServerType");
  GLADE_HOOKUP_OBJECT (AddConnection, hseparator3, "hseparator3");
  GLADE_HOOKUP_OBJECT (AddConnection, label14, "label14");
  GLADE_HOOKUP_OBJECT (AddConnection, DisableAsync, "DisableAsync");
  GLADE_HOOKUP_OBJECT_NO_REF (AddConnection, dialog_action_area2, "dialog_action_area2");
  GLADE_HOOKUP_OBJECT (AddConnection, cancelbutton2, "cancelbutton2");
  GLADE_HOOKUP_OBJECT (AddConnection, okbutton2, "okbutton2");

  return AddConnection;
}

GtkWidget*
create_RemoteMachine (void)
{
  GtkWidget *RemoteMachine;
  GtkWidget *dialog_vbox3;
  GtkWidget *table1;
  GtkWidget *label6;
  GtkWidget *RemoteMachinePort;
  GtkWidget *label11;
  GtkWidget *RemoteMachineHostname;
  GtkWidget *dialog_action_area3;
  GtkWidget *cancelbutton3;
  GtkWidget *okbutton3;

  RemoteMachine = gtk_dialog_new ();
  gtk_widget_set_name (RemoteMachine, "RemoteMachine");
  gtk_window_set_title (GTK_WINDOW (RemoteMachine), "Configure Remote Machine");
  gtk_window_set_modal (GTK_WINDOW (RemoteMachine), TRUE);

  dialog_vbox3 = GTK_DIALOG (RemoteMachine)->vbox;
  gtk_widget_set_name (dialog_vbox3, "dialog_vbox3");
  gtk_widget_show (dialog_vbox3);

  table1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_set_name (table1, "table1");
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox3), table1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

  label6 = gtk_label_new_with_mnemonic ("_Port:");
  gtk_widget_set_name (label6, "label6");
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  RemoteMachinePort = gtk_entry_new ();
  gtk_widget_set_name (RemoteMachinePort, "RemoteMachinePort");
  gtk_widget_show (RemoteMachinePort);
  gtk_table_attach (GTK_TABLE (table1), RemoteMachinePort, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label11 = gtk_label_new_with_mnemonic ("_Hostname:");
  gtk_widget_set_name (label11, "label11");
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table1), label11, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label11), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label11), 0, 0.5);

  RemoteMachineHostname = gtk_entry_new ();
  gtk_widget_set_name (RemoteMachineHostname, "RemoteMachineHostname");
  gtk_widget_show (RemoteMachineHostname);
  gtk_table_attach (GTK_TABLE (table1), RemoteMachineHostname, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  dialog_action_area3 = GTK_DIALOG (RemoteMachine)->action_area;
  gtk_widget_set_name (dialog_action_area3, "dialog_action_area3");
  gtk_widget_show (dialog_action_area3);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area3), GTK_BUTTONBOX_END);

  cancelbutton3 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_set_name (cancelbutton3, "cancelbutton3");
  gtk_widget_show (cancelbutton3);
  gtk_dialog_add_action_widget (GTK_DIALOG (RemoteMachine), cancelbutton3, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton3, GTK_CAN_DEFAULT);

  okbutton3 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_set_name (okbutton3, "okbutton3");
  gtk_widget_show (okbutton3);
  gtk_dialog_add_action_widget (GTK_DIALOG (RemoteMachine), okbutton3, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton3, GTK_CAN_DEFAULT);

  g_signal_connect_swapped ((gpointer) cancelbutton3, "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (RemoteMachine));
  g_signal_connect_swapped ((gpointer) okbutton3, "clicked",
                            G_CALLBACK (GTK_add_tcp_server),
                            GTK_OBJECT (RemoteMachine));

  gtk_label_set_mnemonic_widget (GTK_LABEL (label6), RemoteMachinePort);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label11), RemoteMachineHostname);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (RemoteMachine, RemoteMachine, "RemoteMachine");
  GLADE_HOOKUP_OBJECT_NO_REF (RemoteMachine, dialog_vbox3, "dialog_vbox3");
  GLADE_HOOKUP_OBJECT (RemoteMachine, table1, "table1");
  GLADE_HOOKUP_OBJECT (RemoteMachine, label6, "label6");
  GLADE_HOOKUP_OBJECT (RemoteMachine, RemoteMachinePort, "RemoteMachinePort");
  GLADE_HOOKUP_OBJECT (RemoteMachine, label11, "label11");
  GLADE_HOOKUP_OBJECT (RemoteMachine, RemoteMachineHostname, "RemoteMachineHostname");
  GLADE_HOOKUP_OBJECT_NO_REF (RemoteMachine, dialog_action_area3, "dialog_action_area3");
  GLADE_HOOKUP_OBJECT (RemoteMachine, cancelbutton3, "cancelbutton3");
  GLADE_HOOKUP_OBJECT (RemoteMachine, okbutton3, "okbutton3");

  return RemoteMachine;
}

GtkWidget*
create_LocalExecutable (void)
{
  GtkWidget *LocalExecutable;
  GtkWidget *dialog_vbox4;
  GtkWidget *vbox3;
  GtkWidget *label12;
  GtkWidget *hbox6;
  GtkWidget *LocalExecutablePath;
  GtkWidget *button6;
  GtkWidget *dialog_action_area4;
  GtkWidget *cancelbutton4;
  GtkWidget *okbutton4;

  LocalExecutable = gtk_dialog_new ();
  gtk_widget_set_name (LocalExecutable, "LocalExecutable");
  gtk_window_set_title (GTK_WINDOW (LocalExecutable), "Configure Local Executable");
  gtk_window_set_modal (GTK_WINDOW (LocalExecutable), TRUE);

  dialog_vbox4 = GTK_DIALOG (LocalExecutable)->vbox;
  gtk_widget_set_name (dialog_vbox4, "dialog_vbox4");
  gtk_widget_show (dialog_vbox4);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox3, "vbox3");
  gtk_widget_show (vbox3);
  gtk_box_pack_start (GTK_BOX (dialog_vbox4), vbox3, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox3), 5);

  label12 = gtk_label_new ("Please enter name of server executable");
  gtk_widget_set_name (label12, "label12");
  gtk_widget_show (label12);
  gtk_box_pack_start (GTK_BOX (vbox3), label12, FALSE, FALSE, 10);
  gtk_label_set_justify (GTK_LABEL (label12), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label12), 0, 0.5);

  hbox6 = gtk_hbox_new (FALSE, 5);
  gtk_widget_set_name (hbox6, "hbox6");
  gtk_widget_show (hbox6);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox6, TRUE, TRUE, 0);

  LocalExecutablePath = gtk_entry_new ();
  gtk_widget_set_name (LocalExecutablePath, "LocalExecutablePath");
  gtk_widget_show (LocalExecutablePath);
  gtk_box_pack_start (GTK_BOX (hbox6), LocalExecutablePath, TRUE, TRUE, 0);

  button6 = gtk_button_new_with_mnemonic ("_Browse...");
  gtk_widget_set_name (button6, "button6");
  gtk_widget_show (button6);
  gtk_box_pack_start (GTK_BOX (hbox6), button6, FALSE, FALSE, 0);

  dialog_action_area4 = GTK_DIALOG (LocalExecutable)->action_area;
  gtk_widget_set_name (dialog_action_area4, "dialog_action_area4");
  gtk_widget_show (dialog_action_area4);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area4), GTK_BUTTONBOX_END);

  cancelbutton4 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_set_name (cancelbutton4, "cancelbutton4");
  gtk_widget_show (cancelbutton4);
  gtk_dialog_add_action_widget (GTK_DIALOG (LocalExecutable), cancelbutton4, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton4, GTK_CAN_DEFAULT);

  okbutton4 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_set_name (okbutton4, "okbutton4");
  gtk_widget_show (okbutton4);
  gtk_dialog_add_action_widget (GTK_DIALOG (LocalExecutable), okbutton4, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton4, GTK_CAN_DEFAULT);

  g_signal_connect_swapped ((gpointer) button6, "clicked",
                            G_CALLBACK (GTK_browse_server_executable),
                            GTK_OBJECT (LocalExecutablePath));
  g_signal_connect_swapped ((gpointer) cancelbutton4, "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (LocalExecutable));
  g_signal_connect_swapped ((gpointer) okbutton4, "clicked",
                            G_CALLBACK (GTK_add_file_server),
                            GTK_OBJECT (LocalExecutablePath));

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (LocalExecutable, LocalExecutable, "LocalExecutable");
  GLADE_HOOKUP_OBJECT_NO_REF (LocalExecutable, dialog_vbox4, "dialog_vbox4");
  GLADE_HOOKUP_OBJECT (LocalExecutable, vbox3, "vbox3");
  GLADE_HOOKUP_OBJECT (LocalExecutable, label12, "label12");
  GLADE_HOOKUP_OBJECT (LocalExecutable, hbox6, "hbox6");
  GLADE_HOOKUP_OBJECT (LocalExecutable, LocalExecutablePath, "LocalExecutablePath");
  GLADE_HOOKUP_OBJECT (LocalExecutable, button6, "button6");
  GLADE_HOOKUP_OBJECT_NO_REF (LocalExecutable, dialog_action_area4, "dialog_action_area4");
  GLADE_HOOKUP_OBJECT (LocalExecutable, cancelbutton4, "cancelbutton4");
  GLADE_HOOKUP_OBJECT (LocalExecutable, okbutton4, "okbutton4");

  return LocalExecutable;
}

GtkWidget*
create_ConnectTerminal (void)
{
  GtkWidget *ConnectTerminal;
  GtkWidget *scrolledwindow2;
  GtkWidget *textview1;

  ConnectTerminal = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (ConnectTerminal, "ConnectTerminal");
  gtk_window_set_title (GTK_WINDOW (ConnectTerminal), "GtkHack: Failed connection");
  gtk_window_set_modal (GTK_WINDOW (ConnectTerminal), TRUE);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow2, "scrolledwindow2");
  gtk_widget_show (scrolledwindow2);
  gtk_container_add (GTK_CONTAINER (ConnectTerminal), scrolledwindow2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  textview1 = gtk_text_view_new ();
  gtk_widget_set_name (textview1, "textview1");
  gtk_widget_show (textview1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), textview1);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview1), GTK_WRAP_CHAR);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (ConnectTerminal, ConnectTerminal, "ConnectTerminal");
  GLADE_HOOKUP_OBJECT (ConnectTerminal, scrolledwindow2, "scrolledwindow2");
  GLADE_HOOKUP_OBJECT (ConnectTerminal, textview1, "textview1");

  return ConnectTerminal;
}

