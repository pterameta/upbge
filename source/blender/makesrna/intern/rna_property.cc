/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): Blender Foundation (2008).
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/makesrna/intern/rna_property.c
 *  \ingroup RNA
 */

#include <stdlib.h>

#include "DNA_object_types.h"
#include "DNA_property_types.h"

#include "BLI_path_util.h"
#include "BLI_string_utils.h"

#include "BLT_translation.h"

#include "RNA_define.h"
#include "RNA_enum_types.h"

#include "rna_internal.h"

#include "WM_types.h"

const EnumPropertyItem rna_enum_gameproperty_type_items[] = {
    {GPROP_BOOL, "BOOL", 0, "Boolean", "Boolean Property"},
    {GPROP_INT, "INT", 0, "Integer", "Integer Property"},
    {GPROP_FLOAT, "FLOAT", 0, "Float", "Floating-Point Property"},
    {GPROP_STRING, "STRING", 0, "String", "String Property"},
    {GPROP_TIME, "TIMER", 0, "Timer", "Timer Property"},
    {0, nullptr, 0, nullptr, nullptr}};

#ifdef RNA_RUNTIME

#  include "BKE_property.h"

static StructRNA *rna_GameProperty_refine(struct PointerRNA *ptr)
{
  bProperty *property = (bProperty *)ptr->data;

  switch (property->type) {
    case GPROP_BOOL:
      return &RNA_GameBooleanProperty;
    case GPROP_INT:
      return &RNA_GameIntProperty;
    case GPROP_FLOAT:
      return &RNA_GameFloatProperty;
    case GPROP_STRING:
      return &RNA_GameStringProperty;
    case GPROP_TIME:
      return &RNA_GameTimerProperty;
    default:
      return &RNA_GameProperty;
  }
}

/* for both float and timer */
static float rna_GameFloatProperty_value_get(PointerRNA *ptr)
{
  bProperty *prop = (bProperty *)(ptr->data);
  return *(float *)(&prop->data);
}

static void rna_GameFloatProperty_value_set(PointerRNA *ptr, float value)
{
  bProperty *prop = (bProperty *)(ptr->data);
  CLAMP(value, -FLT_MAX, FLT_MAX);
  *(float *)(&prop->data) = value;
}

static void rna_GameProperty_type_set(PointerRNA *ptr, int value)
{
  bProperty *prop = (bProperty *)(ptr->data);

  if (prop->type != value) {
    prop->type = value;
    BKE_bproperty_init(prop);
  }
}

static void rna_GameProperty_name_set(PointerRNA *ptr, const char *value)
{
  Object *ob = (Object *)ptr->owner_id;
  bProperty *prop = (bProperty *)ptr->data;
  BLI_strncpy_utf8(prop->name, value, sizeof(prop->name));

  BLI_uniquename(
      &ob->prop, prop, DATA_("Property"), '.', offsetof(bProperty, name), sizeof(prop->name));
}

static void rna_GameStringProperty_value_get(PointerRNA *ptr, char *value)
{
  bProperty *prop = (bProperty *)ptr->data;
  if (prop->poin) {
    strcpy(value, (const char *)prop->poin);
  }
  else {
    value[0] = '\0';
  }
}

static int rna_GameStringProperty_value_length(PointerRNA *ptr)
{
  bProperty *prop = (bProperty *)ptr->data;

  if (prop->poin) {
    return strlen((const char *)prop->poin);
  }
  else {
    return 0;
  }
}

static void rna_GameStringProperty_value_set(PointerRNA *ptr, const char *value)
{
  bProperty *prop = (bProperty *)ptr->data;

  if (prop->poin) {
    MEM_freeN(prop->poin);
  }

  if (value[0]) {
    prop->poin = BLI_strdup(value);
  }
  else {
    prop->poin = nullptr;
  }
}

#else

void RNA_def_gameproperty(BlenderRNA *brna)
{
  StructRNA *srna;
  PropertyRNA *prop;

  /* Base Struct for GameProperty */
  srna = RNA_def_struct(brna, "GameProperty", nullptr);
  RNA_def_struct_ui_text(srna, "Game Property", "Game engine user defined object property");
  RNA_def_struct_sdna(srna, "bProperty");
  RNA_def_struct_refine_func(srna, "rna_GameProperty_refine");

  prop = RNA_def_property(srna, "name", PROP_STRING, PROP_NONE);
  RNA_def_property_ui_text(
      prop, "Name", "Available as GameObject attributes in the game engine's python API");
  RNA_def_struct_name_property(srna, prop);
  RNA_def_property_string_funcs(prop, nullptr, nullptr, "rna_GameProperty_name_set");
  RNA_def_property_update(prop, NC_LOGIC, nullptr);

  prop = RNA_def_property(srna, "type", PROP_ENUM, PROP_NONE);
  RNA_def_property_enum_items(prop, rna_enum_gameproperty_type_items);
  RNA_def_property_ui_text(prop, "Type", "");
  RNA_def_property_enum_funcs(prop, nullptr, "rna_GameProperty_type_set", nullptr);
  RNA_def_property_update(prop, NC_LOGIC, nullptr);

  prop = RNA_def_property(srna, "show_debug", PROP_BOOLEAN, PROP_NONE);
  RNA_def_property_boolean_sdna(prop, nullptr, "flag", PROP_DEBUG);
  RNA_def_property_ui_text(prop, "Debug", "Print debug information for this property");
  RNA_def_property_update(prop, NC_LOGIC, nullptr);

  /* GameBooleanProperty */
  srna = RNA_def_struct(brna, "GameBooleanProperty", "GameProperty");
  RNA_def_struct_ui_text(
      srna, "Game Boolean Property", "Game engine user defined Boolean property");
  RNA_def_struct_sdna(srna, "bProperty");
  RNA_def_property_update(prop, NC_LOGIC, nullptr);

  prop = RNA_def_property(srna, "value", PROP_BOOLEAN, PROP_NONE);
  RNA_def_property_boolean_sdna(prop, nullptr, "data", 1);
  RNA_def_property_ui_text(prop, "Value", "Property value");
  RNA_def_property_update(prop, NC_LOGIC, nullptr);

  /* GameIntProperty */
  srna = RNA_def_struct(brna, "GameIntProperty", "GameProperty");
  RNA_def_struct_ui_text(
      srna, "Game Integer Property", "Game engine user defined integer number property");
  RNA_def_struct_sdna(srna, "bProperty");

  prop = RNA_def_property(srna, "value", PROP_INT, PROP_NONE);
  RNA_def_property_int_sdna(prop, nullptr, "data");
  RNA_def_property_ui_text(prop, "Value", "Property value");
  RNA_def_property_range(prop, -INT_MAX, INT_MAX);
  RNA_def_property_update(prop, NC_LOGIC, nullptr);

  /* GameFloatProperty */
  srna = RNA_def_struct(brna, "GameFloatProperty", "GameProperty");
  RNA_def_struct_ui_text(
      srna, "Game Float Property", "Game engine user defined floating point number property");
  RNA_def_struct_sdna(srna, "bProperty");

  prop = RNA_def_property(srna, "value", PROP_FLOAT, PROP_NONE);
  /* RNA_def_property_float_sdna(prop, nullptr, "data"); */
  RNA_def_property_ui_text(prop, "Value", "Property value");
  RNA_def_property_range(prop, -FLT_MAX, FLT_MAX);
  RNA_def_property_float_funcs(
      prop, "rna_GameFloatProperty_value_get", "rna_GameFloatProperty_value_set", nullptr);
  RNA_def_property_update(prop, NC_LOGIC, nullptr);

  /* GameTimerProperty */
  srna = RNA_def_struct(brna, "GameTimerProperty", "GameProperty");
  RNA_def_struct_ui_text(srna, "Game Timer Property", "Game engine user defined timer property");
  RNA_def_struct_sdna(srna, "bProperty");

  prop = RNA_def_property(srna, "value", PROP_FLOAT, PROP_NONE);
  /* RNA_def_property_float_sdna(prop, nullptr, "data"); */
  RNA_def_property_ui_text(prop, "Value", "Property value");
  RNA_def_property_range(prop, -FLT_MAX, FLT_MAX);
  RNA_def_property_float_funcs(
      prop, "rna_GameFloatProperty_value_get", "rna_GameFloatProperty_value_set", nullptr);
  RNA_def_property_update(prop, NC_LOGIC, nullptr);

  /* GameStringProperty */
  srna = RNA_def_struct(brna, "GameStringProperty", "GameProperty");
  RNA_def_struct_ui_text(
      srna, "Game String Property", "Game engine user defined text string property");
  RNA_def_struct_sdna(srna, "bProperty");

  prop = RNA_def_property(srna, "value", PROP_STRING, PROP_NONE);
  RNA_def_property_string_sdna(prop, nullptr, "poin");
  RNA_def_property_string_maxlength(prop, MAX_PROPSTRING);
  RNA_def_property_string_funcs(prop,
                                "rna_GameStringProperty_value_get",
                                "rna_GameStringProperty_value_length",
                                "rna_GameStringProperty_value_set");
  RNA_def_property_ui_text(prop, "Value", "Property value");
  RNA_def_property_update(prop, NC_LOGIC, nullptr);
}

#endif
