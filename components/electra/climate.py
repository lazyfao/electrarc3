import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, remote_receiver
# --- НАЧАЛО ИСПРАВЛЕНИЯ ---
# Эти константы находятся в пространстве имен 'climate', а не в глобальном 'const'
from esphome.components.climate.const import (
    CONF_REMOTE_RECEIVER_ID,
    CONF_REMOTE_TRANSMITTER_ID,
)
# А эта константа действительно глобальная
from esphome.const import CONF_ID
# --- КОНЕЦ ИСПРАВЛЕНИЯ ---


cv.require_ext_library("crankyoldgit/IRremoteESP8266 @ ^2.8.4")

electra_ac_ns = cg.esphome_ns.namespace("electra_ac")
ElectraACClimate = electra_ac_ns.class_("ElectraACClimate", climate.Climate, cg.Component)

# Остальная часть файла остается БЕЗ ИЗМЕНЕНИЙ, так как она уже использует
# правильно импортированные переменные.

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(ElectraACClimate),
        cv.Required(CONF_REMOTE_TRANSMITTER_ID): cv.use_id(
            climate.remote_base.RemoteTransmitterBase
        ),
        cv.Optional(CONF_REMOTE_RECEIVER_ID): cv.use_id(
            remote_receiver.RemoteReceiverComponent
        ),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    transmitter = await cg.get_variable(config[CONF_REMOTE_TRANSMITTER_ID])
    cg.add(var.set_transmitter(transmitter))

    if CONF_REMOTE_RECEIVER_ID in config:
        receiver = await cg.get_variable(config[CONF_REMOTE_RECEIVER_ID])
        cg.add(var.set_receiver(receiver))
