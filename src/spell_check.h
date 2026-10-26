#ifndef SPELL_CHECK_H
#define SPELL_CHECK_H

void spell_check_buffer_full(GtkTextBuffer *tb, SpellCheckData *data);

void tv_enable_spell_check(GtkTextView *tv, SpellCheckData *data);

SpellCheckData *spell_checker_new(const char *dict_path);

#endif // SPELL_CHECK_H
