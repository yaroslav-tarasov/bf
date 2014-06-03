#ifndef BARRIER_INTEGRATION_H
#define BARRIER_INTEGRATION_H

static const uint16_t BAU_FW_RULE_EXISTS =            5400; /* Правило уже присутствует (ошибка/предупреждение) */
static const uint16_t BAU_FW_RULE_ABSENT =            5401; /* Правило отсутствует (ошибка/предупреждение)*/
static const uint16_t BAU_FW_RULE_ADDED =             5402; /* Правило добавлено */
static const uint16_t BAU_FW_RULE_DELETED =           5403; /* Правило удалено */
static const uint16_t BAU_FW_RULES_FLUSHED =          5404; /* Все правила удалены */
static const uint16_t BAU_FW_RULE_ON =                5405; /* Правило выключено */
static const uint16_t BAU_FW_RULE_OFF =               5406; /* Правило включено */
static const uint16_t BAU_FW_CHAIN_POLICY_CHANGED =   5407; /* Правило цепочки */

#endif // BARRIER_INTEGRATION_H

