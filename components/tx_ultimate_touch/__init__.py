import esphome.codegen           as cg
import esphome.config_validation as cv
from   esphome.components    import uart
from   esphome.const         import (
    CONF_ABOVE,
    CONF_BELOW,
    CONF_ID,
    CONF_TRIGGER_ID,
    CONF_UART_ID,
)

from esphome import automation

CODEOWNERS                  = ["@abangtor"]
DEPENDENCIES                = ['uart']

# CONFIG-IDs
CONF_TX_ULTIMATE_TOUCH      = "tx_ultimate_touch"

CONF_UART                   = "uart"

CONF_ON_TOUCH_EVENT         = "on_touch_event"
CONF_ON_PRESS               = "on_press"
CONF_ON_RELEASE             = "on_release"
CONF_ON_DASH                = "on_dash"
CONF_ON_DASH_LEFT           = "on_dash_left"
CONF_ON_DASH_RIGHT          = "on_dash_right"
CONF_ON_SWIPE               = "on_swipe"
CONF_ON_SWIPE_LEFT          = "on_swipe_left"
CONF_ON_SWIPE_RIGHT         = "on_swipe_right"
CONF_ON_MULTI_TOUCH_RELEASE = "on_multi_touch_release"
CONF_ON_LONG_TOUCH_RELEASE  = "on_long_touch_release"
CONF_ON_LONG_PRESS_TIME     = "on_long_press_time"
CONF_ON_UNKNOWN_FRAME       = "on_unknown_frame"
CONF_LONG_PRESS_X_OFFSET    = "long_press_x_offset"
CONF_VALIDATE_CRC           = "validate_crc"

# ------------------------------
#  Touch Config
# ------------------------------

tx_ultimate_touch_ns = cg.esphome_ns.namespace('tx_ultimate_touch')
TouchPoint = tx_ultimate_touch_ns.struct("TouchPoint")
TouchGesture = tx_ultimate_touch_ns.struct("TouchGesture")
TouchFrame = tx_ultimate_touch_ns.struct("TouchFrame")

TxUltimateTouch = tx_ultimate_touch_ns.class_(
    'TxUltimateTouch', cg.Component, uart.UARTDevice)
LongPressTimeTrigger = tx_ultimate_touch_ns.class_(
    "LongPressTimeTrigger", automation.Trigger.template(cg.uint32)
)

# ------------------------------
#  Parameter Config
# ------------------------------
def _migrate_legacy_uart(config):
    config = config.copy()
    if CONF_UART in config and CONF_UART_ID in config:
        raise cv.Invalid("Use either 'uart_id' or legacy 'uart', not both")
    if CONF_UART in config:
        config[CONF_UART_ID] = config.pop(CONF_UART)
    return config


def _time_period_milliseconds(value):
    return cv.positive_time_period_milliseconds(value).total_milliseconds


CONFIG_SCHEMA = cv.All(_migrate_legacy_uart, cv.Schema({
    cv.GenerateID():                          cv.declare_id(TxUltimateTouch),

    cv.Optional(CONF_LONG_PRESS_X_OFFSET, default=16): cv.int_range(min=1, max=127),
    cv.Optional(CONF_VALIDATE_CRC, default=True):      cv.boolean,

    cv.Optional(CONF_ON_TOUCH_EVENT):         automation.validate_automation(single=True),
    cv.Optional(CONF_ON_PRESS):               automation.validate_automation(single=True),
    cv.Optional(CONF_ON_RELEASE):             automation.validate_automation(single=True),
    cv.Optional(CONF_ON_DASH):                automation.validate_automation(single=True),
    cv.Optional(CONF_ON_DASH_LEFT):           automation.validate_automation(single=True),
    cv.Optional(CONF_ON_DASH_RIGHT):          automation.validate_automation(single=True),
    cv.Optional(CONF_ON_SWIPE):               automation.validate_automation(single=True),
    cv.Optional(CONF_ON_SWIPE_LEFT):          automation.validate_automation(single=True),
    cv.Optional(CONF_ON_SWIPE_RIGHT):         automation.validate_automation(single=True),
    cv.Optional(CONF_ON_MULTI_TOUCH_RELEASE): automation.validate_automation(single=True),
    cv.Optional(CONF_ON_LONG_TOUCH_RELEASE):  automation.validate_automation(single=True),
    cv.Optional(CONF_ON_LONG_PRESS_TIME):     automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(LongPressTimeTrigger),
            cv.Optional(CONF_ABOVE): _time_period_milliseconds,
            cv.Optional(CONF_BELOW): _time_period_milliseconds,
        },
        cv.has_at_least_one_key(CONF_ABOVE, CONF_BELOW),
    ),
    cv.Optional(CONF_ON_UNKNOWN_FRAME):       automation.validate_automation(single=True),

}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA))


# ------------------------------
#  Actions
# ------------------------------
async def register_tx_ultimate_touch(var, config):
    cg.add(var.set_long_press_x_offset(config[CONF_LONG_PRESS_X_OFFSET]))
    cg.add(var.set_validate_crc(config[CONF_VALIDATE_CRC]))

    if CONF_ON_TOUCH_EVENT in config:
        await automation.build_automation(
            var.get_trigger_touch_event(),
            [(TouchPoint, "touch")],
            config[CONF_ON_TOUCH_EVENT],
        )

    if CONF_ON_PRESS in config:
        await automation.build_automation(
            var.get_trigger_touch(),
            [(TouchPoint, "touch")],
            config[CONF_ON_PRESS],
        )

    if CONF_ON_RELEASE in config:
        await automation.build_automation(
            var.get_trigger_release(),
            [(TouchPoint, "touch")],
            config[CONF_ON_RELEASE],
        )

    if CONF_ON_DASH in config:
        await automation.build_automation(
            var.get_trigger_dash(),
            [(TouchGesture, "gesture")],
            config[CONF_ON_DASH],
        )

    if CONF_ON_DASH_LEFT in config:
        await automation.build_automation(
            var.get_trigger_dash_left(),
            [(TouchGesture, "gesture")],
            config[CONF_ON_DASH_LEFT],
        )

    if CONF_ON_DASH_RIGHT in config:
        await automation.build_automation(
            var.get_trigger_dash_right(),
            [(TouchGesture, "gesture")],
            config[CONF_ON_DASH_RIGHT],
        )

    if CONF_ON_SWIPE in config:
        await automation.build_automation(
            var.get_trigger_swipe(),
            [(TouchGesture, "gesture")],
            config[CONF_ON_SWIPE],
        )

    if CONF_ON_SWIPE_LEFT in config:
        await automation.build_automation(
            var.get_trigger_swipe_left(),
            [(TouchPoint, "touch")],
            config[CONF_ON_SWIPE_LEFT],
        )

    if CONF_ON_SWIPE_RIGHT in config:
        await automation.build_automation(
            var.get_trigger_swipe_right(),
            [(TouchPoint, "touch")],
            config[CONF_ON_SWIPE_RIGHT],
        )

    if CONF_ON_MULTI_TOUCH_RELEASE in config:
        await automation.build_automation(
            var.get_trigger_multi_touch_release(),
            [(TouchPoint, "touch")],
            config[CONF_ON_MULTI_TOUCH_RELEASE],
        )

    if CONF_ON_LONG_TOUCH_RELEASE in config:
        await automation.build_automation(
            var.get_trigger_long_touch_release(),
            [(TouchPoint, "touch")],
            config[CONF_ON_LONG_TOUCH_RELEASE],
        )

    for conf in config.get(CONF_ON_LONG_PRESS_TIME, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        if CONF_ABOVE in conf:
            cg.add(trigger.set_min(conf[CONF_ABOVE]))
        if CONF_BELOW in conf:
            cg.add(trigger.set_max(conf[CONF_BELOW]))
        await automation.build_automation(
            trigger,
            [(cg.uint32, "duration_ms")],
            conf,
        )

    if CONF_ON_UNKNOWN_FRAME in config:
        await automation.build_automation(
            var.get_trigger_unknown_frame(),
            [(TouchFrame, "frame")],
            config[CONF_ON_UNKNOWN_FRAME],
        )

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    await register_tx_ultimate_touch(var, config)
