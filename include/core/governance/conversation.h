/**
 * @file conversation.h
 * @brief Dialogue and Conversation system
 */

#ifndef CIVILIZATION_CONVERSATION_H
#define CIVILIZATION_CONVERSATION_H

#include "../../common.h"
#include "../../types.h"
#include "interaction.h"

/* Dialogue Option */
typedef struct {
  char text[STRING_MEDIUM_LEN];
  char target_node_id[STRING_SHORT_LEN];
  civ_float_t relationship_effect;
  civ_float_t cost; /* e.g. Influence cost */
} civ_dialogue_option_t;

/* Dialogue Node */
typedef struct {
  char id[STRING_SHORT_LEN];
  char text[STRING_MAX_LEN];

  civ_dialogue_option_t *options;
  size_t option_count;
  size_t option_capacity;

  char script_action[STRING_SHORT_LEN]; /* Action to trigger, e.g.
                                           "START_REFORM" */
} civ_dialogue_node_t;

/* Conversation State */
typedef struct {
  civ_interaction_t *interaction;
  civ_dialogue_node_t *current_node;

  bool active;
  char history[STRING_MAX_LEN];
} civ_conversation_t;

/* Functions */
civ_conversation_t *civ_conversation_start(civ_interaction_t *interaction,
                                           civ_dialogue_node_t *root);
void civ_conversation_destroy(civ_conversation_t *conv);

civ_result_t civ_conversation_select_option(civ_conversation_t *conv,
                                            size_t option_idx);
const char *civ_conversation_get_current_text(const civ_conversation_t *conv);

#endif /* CIVILIZATION_CONVERSATION_H */
