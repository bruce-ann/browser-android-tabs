// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/accessibility/platform/ax_platform_atk_hyperlink.h"

#include "ui/accessibility/ax_text_utils.h"
#include "ui/accessibility/platform/ax_platform_node_auralinux.h"
#include "ui/accessibility/platform/ax_platform_node_delegate.h"

namespace ui {

static gpointer ax_platform_atk_hyperlink_parent_class = nullptr;

static AXPlatformNodeAuraLinux* ToAXPlatformNodeAuraLinux(
    AXPlatformAtkHyperlink* atk_hyperlink) {
  if (!atk_hyperlink)
    return nullptr;

  return atk_hyperlink->m_object;
}

static void ax_platform_atk_hyperlink_finalize(GObject* atk_hyperlink) {
  G_OBJECT_CLASS(ax_platform_atk_hyperlink_parent_class)
      ->finalize(atk_hyperlink);
}

static gchar* ax_platform_atk_hyperlink_get_uri(AtkHyperlink* atk_hyperlink,
                                                gint index) {
  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinux(AX_PLATFORM_ATK_HYPERLINK(atk_hyperlink));
  if (!obj)
    return nullptr;

  if (index != 0)
    return nullptr;

  return g_strdup(obj->GetStringAttribute(ui::AX_ATTR_URL).c_str());
}

static AtkObject* ax_platform_atk_hyperlink_get_object(
    AtkHyperlink* atk_hyperlink,
    gint index) {
  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinux(AX_PLATFORM_ATK_HYPERLINK(atk_hyperlink));
  if (!obj)
    return nullptr;

  if (index != 0)
    return nullptr;

  return ATK_OBJECT(obj->GetNativeViewAccessible());
}

static gint ax_platform_atk_hyperlink_get_n_anchors(
    AtkHyperlink* atk_hyperlink) {
  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinux(AX_PLATFORM_ATK_HYPERLINK(atk_hyperlink));

  return obj ? 1 : 0;
}

static gboolean ax_platform_atk_hyperlink_is_valid(
    AtkHyperlink* atk_hyperlink) {
  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinux(AX_PLATFORM_ATK_HYPERLINK(atk_hyperlink));

  return obj ? TRUE : FALSE;
}

static gboolean ax_platform_atk_hyperlink_is_selected_link(
    AtkHyperlink* atk_hyperlink) {
  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinux(AX_PLATFORM_ATK_HYPERLINK(atk_hyperlink));

  if (!obj)
    return false;

  return obj->GetDelegate()->GetFocus() == obj->GetNativeViewAccessible();
}

static void ax_platform_atk_hyperlink_class_init(AtkHyperlinkClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  ax_platform_atk_hyperlink_parent_class = g_type_class_peek_parent(klass);

  gobject_class->finalize = ax_platform_atk_hyperlink_finalize;
  klass->get_uri = ax_platform_atk_hyperlink_get_uri;
  klass->get_object = ax_platform_atk_hyperlink_get_object;
  klass->is_valid = ax_platform_atk_hyperlink_is_valid;
  klass->get_n_anchors = ax_platform_atk_hyperlink_get_n_anchors;
  klass->is_selected_link = ax_platform_atk_hyperlink_is_selected_link;
  // TODO(jose.dapena) implement get_start_index and get_end_index methods
  // that should provide the range of the link in the embedding text.
}

//
// AtkAction interface.
//

static AXPlatformNodeAuraLinux* ToAXPlatformNodeAuraLinuxFromHyperlinkAction(
    AtkAction* atk_action) {
  if (!IS_AX_PLATFORM_ATK_HYPERLINK(atk_action))
    return nullptr;

  return ToAXPlatformNodeAuraLinux(AX_PLATFORM_ATK_HYPERLINK(atk_action));
}

static gboolean ax_platform_atk_hyperlink_do_action(AtkAction* action,
                                                    gint index) {
  g_return_val_if_fail(ATK_IS_ACTION(action), FALSE);
  g_return_val_if_fail(!index, FALSE);

  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinuxFromHyperlinkAction(action);
  if (!obj)
    return FALSE;

  obj->DoDefaultAction();

  return TRUE;
}

static gint ax_platform_atk_hyperlink_get_n_actions(AtkAction* action) {
  g_return_val_if_fail(ATK_IS_ACTION(action), FALSE);

  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinuxFromHyperlinkAction(action);
  if (!obj)
    return 0;

  return 1;
}

static const gchar* ax_platform_atk_hyperlink_get_description(AtkAction* action,
                                                              gint index) {
  g_return_val_if_fail(ATK_IS_ACTION(action), FALSE);
  g_return_val_if_fail(!index, FALSE);

  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinuxFromHyperlinkAction(action);
  if (!obj)
    return nullptr;

  // Not implemented
  return nullptr;
}

static const gchar* ax_platform_atk_hyperlink_get_keybinding(AtkAction* action,
                                                             gint index) {
  g_return_val_if_fail(ATK_IS_ACTION(action), FALSE);
  g_return_val_if_fail(!index, FALSE);

  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinuxFromHyperlinkAction(action);
  if (!obj)
    return nullptr;

  return obj->GetStringAttribute(ui::AX_ATTR_ACCESS_KEY).c_str();
}

static const gchar* ax_platform_atk_hyperlink_get_name(AtkAction* atk_action,
                                                       gint index) {
  g_return_val_if_fail(ATK_IS_ACTION(atk_action), FALSE);
  g_return_val_if_fail(!index, FALSE);

  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinuxFromHyperlinkAction(atk_action);
  if (!obj)
    return nullptr;

  int action;
  if (!obj->GetIntAttribute(ui::AX_ATTR_DEFAULT_ACTION_VERB, &action))
    return nullptr;
  base::string16 action_verb = ui::ActionVerbToUnlocalizedString(
      static_cast<ui::AXDefaultActionVerb>(action));
  ATK_AURALINUX_RETURN_STRING(base::UTF16ToUTF8(action_verb));
}

static const gchar* ax_platform_atk_hyperlink_get_localized_name(
    AtkAction* atk_action,
    gint index) {
  g_return_val_if_fail(ATK_IS_ACTION(atk_action), FALSE);
  g_return_val_if_fail(!index, FALSE);

  AXPlatformNodeAuraLinux* obj =
      ToAXPlatformNodeAuraLinuxFromHyperlinkAction(atk_action);
  if (!obj)
    return nullptr;

  int action;
  if (!obj->GetIntAttribute(ui::AX_ATTR_DEFAULT_ACTION_VERB, &action))
    return nullptr;
  base::string16 action_verb = ui::ActionVerbToLocalizedString(
      static_cast<ui::AXDefaultActionVerb>(action));
  ATK_AURALINUX_RETURN_STRING(base::UTF16ToUTF8(action_verb));
}

static void atk_action_interface_init(AtkActionIface* iface) {
  iface->do_action = ax_platform_atk_hyperlink_do_action;
  iface->get_n_actions = ax_platform_atk_hyperlink_get_n_actions;
  iface->get_description = ax_platform_atk_hyperlink_get_description;
  iface->get_keybinding = ax_platform_atk_hyperlink_get_keybinding;
  iface->get_name = ax_platform_atk_hyperlink_get_name;
  iface->get_localized_name = ax_platform_atk_hyperlink_get_localized_name;
}

void ax_platform_atk_hyperlink_set_object(AXPlatformAtkHyperlink* atk_hyperlink,
                                          AXPlatformNodeAuraLinux* obj) {
  g_return_if_fail(AX_PLATFORM_ATK_HYPERLINK(atk_hyperlink));
  atk_hyperlink->m_object = obj;
}

GType ax_platform_atk_hyperlink_get_type() {
  static volatile gsize type_volatile = 0;

  AXPlatformNodeAuraLinux::EnsureGTypeInit();

  if (g_once_init_enter(&type_volatile)) {
    static const GTypeInfo tinfo = {
        sizeof(AXPlatformAtkHyperlinkClass),
        (GBaseInitFunc) nullptr,
        (GBaseFinalizeFunc) nullptr,
        (GClassInitFunc)ax_platform_atk_hyperlink_class_init,
        (GClassFinalizeFunc) nullptr,
        nullptr,                        /* class data */
        sizeof(AXPlatformAtkHyperlink), /* instance size */
        0,                              /* nb preallocs */
        (GInstanceInitFunc) nullptr,
        nullptr /* value table */
    };

    static const GInterfaceInfo actionInfo = {
        (GInterfaceInitFunc)(GInterfaceInitFunc)atk_action_interface_init,
        (GInterfaceFinalizeFunc)0, 0};

    GType type = g_type_register_static(
        ATK_TYPE_HYPERLINK, "AXPlatformAtkHyperlink", &tinfo, GTypeFlags(0));
    g_type_add_interface_static(type, ATK_TYPE_ACTION, &actionInfo);
    g_once_init_leave(&type_volatile, type);
  }

  return type_volatile;
}

}  // namespace ui
