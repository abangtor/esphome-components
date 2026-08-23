import esphome.codegen as cg
from esphome.components import light
import esphome.config_validation as cv
from esphome.const import (
    CONF_FROM,
    CONF_ID,
    CONF_NUM_LEDS,
    CONF_OUTPUT_ID,
    CONF_REVERSED,
    CONF_SEGMENTS,
    CONF_TO,
)
import esphome.final_validate as fv

CONF_CHANNELS = "channels"

channel_partition_ns = cg.esphome_ns.namespace("channel_partition")
ChannelPartitionSegment = channel_partition_ns.class_("ChannelPartitionSegment")
ChannelPartitionLightOutput = channel_partition_ns.class_(
    "ChannelPartitionLightOutput", light.AddressableLight
)

CHANNELS = {
    "R": 0,
    "G": 1,
    "B": 2,
    "W": 3,
}


def validate_channels(value):
    value = cv.string_strict(value).upper()
    if not value:
        raise cv.Invalid("At least one channel must be provided")
    seen = set()
    for channel in value:
        if channel not in CHANNELS:
            raise cv.Invalid("Channels may only contain R, G, B, and W")
        if channel in seen:
            raise cv.Invalid(f"Channel {channel} is listed more than once")
        seen.add(channel)
    return value


def validate_from_to(value):
    if (CONF_TO in value) == (CONF_NUM_LEDS in value):
        raise cv.Invalid(f"Specify exactly one of {CONF_TO} or {CONF_NUM_LEDS}")
    if CONF_TO in value and value[CONF_FROM] > value[CONF_TO]:
        raise cv.Invalid(
            f"From ({value[CONF_FROM]}) must not be larger than to ({value[CONF_TO]})"
        )
    return value


def validate_segment(config):
    fconf = fv.full_config.get()
    path = fconf.get_path_for_id(config[CONF_ID])[:-1]
    source_light_config = fconf.get_config_for_path(path)

    if CONF_NUM_LEDS in source_light_config:
        source_len = source_light_config[CONF_NUM_LEDS]
        if CONF_NUM_LEDS in config:
            segment_size = config[CONF_NUM_LEDS]
        else:
            segment_size = config[CONF_TO] - config[CONF_FROM] + 1
        segment_to = config[CONF_FROM] + segment_size - 1
        if config[CONF_FROM] >= source_len:
            raise cv.Invalid(
                f"FROM ({config[CONF_FROM]}) must be less than the number of LEDs in light '{config[CONF_ID]}' ({source_len})",
                [CONF_FROM],
            )
        if segment_to >= source_len:
            raise cv.Invalid(
                f"Segment end ({segment_to}) must be less than the number of LEDs in light '{config[CONF_ID]}' ({source_len})",
                [CONF_TO if CONF_TO in config else CONF_NUM_LEDS],
            )


CHANNEL_SEGMENT_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(light.AddressableLightState),
        cv.Required(CONF_FROM): cv.positive_int,
        cv.Optional(CONF_TO): cv.positive_int,
        cv.Optional(CONF_NUM_LEDS): cv.positive_int,
        cv.Required(CONF_CHANNELS): validate_channels,
        cv.Optional(CONF_REVERSED, default=False): cv.boolean,
    }
)

CONFIG_SCHEMA = light.ADDRESSABLE_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(ChannelPartitionLightOutput),
        cv.Required(CONF_SEGMENTS): cv.All(
            cv.ensure_list(CHANNEL_SEGMENT_SCHEMA, validate_from_to),
            cv.Length(min=1),
        ),
    }
)

FINAL_VALIDATE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SEGMENTS): [validate_segment],
    },
    extra=cv.ALLOW_EXTRA,
)


async def to_code(config):
    segments = []
    for conf in config[CONF_SEGMENTS]:
        if CONF_NUM_LEDS in conf:
            segment_size = conf[CONF_NUM_LEDS]
        else:
            segment_size = conf[CONF_TO] - conf[CONF_FROM] + 1
        segments.append(
            ChannelPartitionSegment(
                await cg.get_variable(conf[CONF_ID]),
                conf[CONF_FROM],
                segment_size,
                [CHANNELS[channel] for channel in conf[CONF_CHANNELS]],
                conf[CONF_REVERSED],
            )
        )

    var = cg.new_Pvariable(config[CONF_OUTPUT_ID], segments)
    await cg.register_component(var, config)
    await light.register_light(var, config)
