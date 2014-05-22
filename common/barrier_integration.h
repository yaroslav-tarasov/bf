#ifndef BARRIER_INTEGRATION_H
#define BARRIER_INTEGRATION_H

static const uint16_t BAU_FW_RULE_EXISTS =          5400; /* Правило уже присутствует (ошибка/предупреждение) */
static const uint16_t BAU_FW_RULE_ABSENT =          5400; /* Правило отсутствует (ошибка/предупреждение)*/
static const uint16_t BAU_FW_RULE_ADD =             5401; /* Правило добавлено */
static const uint16_t BAU_FW_RULE_DELETE =          5402; /* Правило удалено */
static const uint16_t BAU_FW_RULES_FLUSH =          5403; /* Все правила удалены */
static const uint16_t BAU_FW_RULE_ON =              5403; /* Правило выключено */
static const uint16_t BAU_FW_RULE_OFF =             5403; /* Правило включено */
static const uint16_t BAU_FW_CHAIN_POLICY =         5403; /* Правило цепочки */

#endif // BARRIER_INTEGRATION_H

