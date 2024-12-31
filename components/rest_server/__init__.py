from __future__ import annotations

import gzip

import esphome.codegen as cg
from esphome.components import web_server_base
from esphome.components.web_server_base import CONF_WEB_SERVER_BASE_ID
import esphome.config_validation as cv
from esphome.const import (
    CONF_AUTH,
    CONF_CSS_INCLUDE,
    CONF_CSS_URL,
    CONF_ENABLE_PRIVATE_NETWORK_ACCESS,
    CONF_ID,
    CONF_INCLUDE_INTERNAL,
    CONF_JS_INCLUDE,
    CONF_JS_URL,
    CONF_LOCAL,
    CONF_LOG,
    CONF_NAME,
    CONF_OTA,
    CONF_PASSWORD,
    CONF_PORT,
    CONF_USERNAME,
    CONF_VERSION,
    CONF_WEB_SERVER,
    CONF_WEB_SERVER_ID,
    PLATFORM_BK72XX,
    PLATFORM_ESP32,
    PLATFORM_ESP8266,
    PLATFORM_RTL87XX,
)
from esphome.core import CORE, coroutine_with_priority
import esphome.final_validate as fv

AUTO_LOAD = ["json", "web_server_base"]

CONF_REST_SERVER    = "rest_server"
CONF_REST_SERVER_ID = "rest_server_id"

rest_server_ns = cg.esphome_ns.namespace("rest_server")
RestServer = rest_server_ns.class_("RestServer", cg.Component, cg.Controller)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(RestServer),
            cv.Optional(CONF_PORT, default=80): cv.port,
            cv.Optional(CONF_ENABLE_PRIVATE_NETWORK_ACCESS, default=True): cv.boolean,
            cv.Optional(CONF_AUTH): cv.Schema(
                {
                    cv.Required(CONF_USERNAME): cv.All(
                        cv.string_strict, cv.Length(min=1)
                    ),
                    cv.Required(CONF_PASSWORD): cv.All(
                        cv.string_strict, cv.Length(min=1)
                    ),
                }
            ),
            cv.GenerateID(CONF_WEB_SERVER_BASE_ID): cv.use_id(
                web_server_base.WebServerBase
            ),
            cv.Optional(CONF_INCLUDE_INTERNAL, default=False): cv.boolean,
            cv.SplitDefault(
                CONF_OTA,
                esp8266=True,
                esp32_arduino=True,
                esp32_idf=False,
                bk72xx=True,
                rtl87xx=True,
            ): cv.boolean
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.only_on([PLATFORM_ESP32, PLATFORM_ESP8266, PLATFORM_BK72XX, PLATFORM_RTL87XX]),
)

async def add_entity_config(entity, config):
    rest_server = await cg.get_variable(config[CONF_REST_SERVER_ID])
    cg.add(
        rest_server.add_entity_config(
            entity
        )
    )

@coroutine_with_priority(40.0)
async def to_code(config):
    paren = await cg.get_variable(config[CONF_WEB_SERVER_BASE_ID])

    var = cg.new_Pvariable(config[CONF_ID], paren)
    await cg.register_component(var, config)

    cg.add(paren.set_port(config[CONF_PORT]))
    cg.add_define("USE_WEBSERVER")
    cg.add_define("USE_WEBSERVER_PORT", config[CONF_PORT])
    cg.add_define("USE_WEBSERVER_VERSION", 1)
    cg.add(var.set_include_internal(config[CONF_INCLUDE_INTERNAL]))
    