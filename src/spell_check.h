#ifndef SPELL_CHECK_H
#define SPELL_CHECK_H

#include <gtk/gtk.h>

typedef struct SpellDict SpellDict;

SpellDict *spell_dict_new(const char *dict_path);

SpellDict *spell_dict_ref(SpellDict *dict);

void spell_dict_unref(SpellDict *dict);

void tv_enable_spell_check(GtkTextView *tv, SpellDict *dict);

void tv_disable_spell_check(GtkTextView *tv);

#endif
