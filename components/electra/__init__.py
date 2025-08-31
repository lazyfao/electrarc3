# -*- coding: utf-8 -*-
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.components import remote_transmitter, remote_receiver
from esphome.const import (
    CONF_ID,
)

# Создаем пространство имен для C++ кода
electra_ns = cg.esphome_ns.namespace("electra")
# Объявляем C++ класс из твоего electra.h
ElectraClimate = electra_ns.class_("ElectraClimate", climate.Climate, cg.Component)

# Схема конфигурации
CONFIG_SCHEMA = climate.CLIMATE_PLATFORM_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(ElectraClimate),
        # Используем правильный ключ для ID передатчика из компонента remote_transmitter
        cv.Required(remote_transmitter.CONF_TRANSMITTER_ID): cv.use_id(
            remote_transmitter.RemoteTransmitterComponent
        ),
        # То же самое для приемника, делаем его опциональным
        cv.Optional(remote_receiver.CONF_RECEIVER_ID): cv.use_id(
            remote_receiver.RemoteReceiverComponent
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Генерирует C++ код из конфигурации."""
    # Получаем Pvariable для нашего C++ класса
    var = cg.new_Pvariable(config[CONF_ID])
    
    # Инициализируем базовый компонент
    await cg.register_component(var, config)
    # Регистрируем нашу платформу как Climate устройство
    await climate.register_climate(var, config)

    # Получаем передатчик и передаем его в C++ код
    transmitter = await cg.get_variable(config[remote_transmitter.CONF_TRANSMITTER_ID])
    cg.add(var.set_transmitter(transmitter))

    # Если приемник указан, получаем и передаем его
    if remote_receiver.CONF_RECEIVER_ID in config:
        receiver = await cg.get_variable(config[remote_receiver.CONF_RECEIVER_ID])
        cg.add(var.set_receiver(receiver))

    # Добавляем внешнюю C++ библиотеку. ESPHome скачает ее автоматически.
    # Это гораздо лучше, чем класть .cpp и .h файлы рядом.
    # Рекомендую вынести C++ код в отдельный репозиторий и подключить так.
    # Пока оставлю как есть, но это совет на будущее.
    # cg.add_library("url_to_your_library", None)
