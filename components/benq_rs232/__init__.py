import esphome.codegen           as cg
import esphome.config_validation as cv

from esphome.automation import Condition
from esphome            import automation
from esphome.components import uart
from esphome.const      import (
    CONF_ID
)


###################
#  Configuration

CODEOWNERS = ["@abangtor"]
DEPENDENCIES = ['uart']

CONF_BENQ_RS232 = "benq_rs232"

CONF_UART = "uart"
CONF_SEND_CMD = "comand"

CONF_ON_REPLY = "on_reply"

benq_rs232_ns = cg.esphome_ns.namespace('benq_rs232')
Reply = benq_rs232_ns.struct("Reply")

SendComandAction = uart_ns.class_("BenqSendCommand", automation.Action)
BenqRs232 = benq_rs232_ns.class_('BenqRs232', cg.Component, uart.UARTDevice)

#######################
#  Config Validation
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID():            cv.declare_id(BenqRs232),
    cv.Required(CONF_UART):     cv.use_id(uart),
    cv.Optional(CONF_ON_REPLY): automation.validate_automation(single=True),
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

#############
#  Actions
async def register_benq_rs232(var, config):
    uart_component = await cg.get_variable(config[CONF_UART])
    cg.add(var.set_uart_component(uart_component))

    if CONF_ON_REPLY in config:
        await automation.build_automation(
            var.get_reply_trigger(),
            [(Reply, "reply")],
            config[CONF_ON_REPLY],
        )

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    await register_benq_rs232(var, config)

@automation.register_action(
    "benq_rs232.send",
    SendComandAction,
    cv.maybe_simple_value(
        {
            cv.GenerateID(): cv.use_id(UARTComponent),
            cv.Required(CONF_SEND_CMD): cv.templatable(validate_raw_data),
        },
        key=CONF_SEND_CMD,
    ),
)
async def uart_write_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    data = config[CONF_SEND_CMD]
    if isinstance(data, bytes):
        data = list(data)

    if cg.is_template(data):
        templ = await cg.templatable(data, args, cg.std_vector.template(cg.uint8))
        cg.add(var.set_data_template(templ))
    else:
        cg.add(var.set_data_static(data))
    return var