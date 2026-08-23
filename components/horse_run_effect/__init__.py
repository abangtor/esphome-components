from esphome import automation
import esphome.codegen as cg
from esphome.components.light.effects import register_addressable_effect
from esphome.components.light.types import AddressableLightEffect
import esphome.config_validation as cv
from esphome.const import CONF_NAME, CONF_UPDATE_INTERVAL

AUTO_LOAD = ["light"]
CONFIG_SCHEMA = cv.Schema({})

horse_run_effect_ns = cg.esphome_ns.namespace("horse_run_effect")
HorseRunEffect = horse_run_effect_ns.class_("HorseRunEffect", AddressableLightEffect)

CONF_FADE_STEPS = "fade_steps"
CONF_ON_FINISHED = "on_finished"
CONF_REVERSE = "reverse"
CONF_TARGET = "target"

TARGETS = {
    "current": False,
    "black": True,
}


@register_addressable_effect(
    "horse_run",
    HorseRunEffect,
    "Horse Run",
    {
        cv.Optional(CONF_UPDATE_INTERVAL, default="25ms"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_FADE_STEPS, default=5): cv.int_range(min=1, max=255),
        cv.Optional(CONF_REVERSE, default=False): cv.boolean,
        cv.Optional(CONF_TARGET, default="current"): cv.one_of(*TARGETS, lower=True),
        cv.Optional(CONF_ON_FINISHED): automation.validate_automation(single=True),
    },
)
async def horse_run_effect_to_code(config, effect_id):
    effect = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(effect.set_update_interval(config[CONF_UPDATE_INTERVAL]))
    cg.add(effect.set_fade_steps(config[CONF_FADE_STEPS]))
    cg.add(effect.set_reverse(config[CONF_REVERSE]))
    cg.add(effect.set_target_black(TARGETS[config[CONF_TARGET]]))

    if CONF_ON_FINISHED in config:
        await automation.build_automation(
            effect.get_finished_trigger(),
            [],
            config[CONF_ON_FINISHED],
        )

    return effect
