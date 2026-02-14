/**
 * @file conversation.c
 * @brief Implementation of dialogue and conversation system
 */

#include "../../../include/core/governance/conversation.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>

civ_conversation_t *civ_conversation_start(civ_interaction_t *interaction,
                                           civ_dialogue_node_t *root) {
  if (!interaction || !root)
    return NULL;

  civ_conversation_t *conv =
      (civ_conversation_t *)CIV_MALLOC(sizeof(civ_conversation_t));
  if (!conv)
    return NULL;

  memset(conv, 0, sizeof(civ_conversation_t));
  conv->interaction = interaction;
  conv->current_node = root;
  conv->active = true;

  civ_interaction_start(interaction);

  return conv;
}

void civ_conversation_destroy(civ_conversation_t *conv) {
  if (!conv)
    return;
  CIV_FREE(conv);
}

civ_result_t civ_conversation_select_option(civ_conversation_t *conv,
                                            size_t option_idx) {
  if (!conv || !conv->active || !conv->current_node)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null conv"};

  if (option_idx >= conv->current_node->option_count)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid option index"};

  civ_dialogue_option_t *opt = &conv->current_node->options[option_idx];

  /* In a real implementation, we would find the next node by ID */
  /* For this version, we'll just log the transition */
  civ_log(CIV_LOG_INFO, "Selected option: %s", opt->text);

  return (civ_result_t){CIV_OK, NULL};
}

const char *civ_conversation_get_current_text(const civ_conversation_t *conv) {
  if (!conv || !conv->current_node)
    return NULL;
  return conv->current_node->text;
}
