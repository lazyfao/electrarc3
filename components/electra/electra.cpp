#include "electra_ac.h"
#include "esphome/core/log.h"

namespace esphome {
namespace electra_ac {

static const char *const TAG = "electra_ac";

// ... (методы setup, dump_config, traits, control, transmit_state_ - без изменений) ...
// Оставь их такими же, как в моем предыдущем ответе. Я их опущу для краткости.

// --- НАЧАЛО ИЗМЕНЕНИЙ ---

void ElectraACClimate::set_receiver(remote_receiver::RemoteReceiverComponent *receiver) {
  receiver->add_on_receive_callback([this](remote_receiver::RemoteReceiveData data) {
    if (this->parse_state_(data)) {
      ESP_LOGD(TAG, "Received valid Electra AC code");
      this->publish_state();
    }
  });
}

bool ElectraACClimate::parse_state_(const remote_receiver::RemoteReceiveData &data) {
  // Пытаемся декодировать RAW данные
  if (!this->ac_.decode(data.get_raw_ptr())) {
    return false; // Это не наш протокол
  }

  // Если код валидный, обновляем состояние компонента
  if (this->ac_.getPower()) {
    switch (this->ac_.getMode()) {
      case kElectraAcCool:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
      case kElectraAcHeat:
        this->mode = climate::CLIMATE_MODE_HEAT;
        break;
      case kElectraAcFan:
        this->mode = climate::CLIMATE_MODE_FAN_ONLY;
        break;
      case kElectraAcDry:
        this->mode = climate::CLIMATE_MODE_DRY;
        break;
      case kElectraAcAuto:
        this->mode = climate::CLIMATE_MODE_AUTO;
        break;
    }
  } else {
    this->mode = climate::CLIMATE_MODE_OFF;
  }

  this->target_temperature = this->ac_.getTemp();

  switch (this->ac_.getFan()) {
    case kElectraAcFanAuto:
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
      break;
    case kElectraAcFanLow:
      this->fan_mode = climate::CLIMATE_FAN_LOW;
      break;
    case kElectraAcFanMedium:
      this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
      break;
    // Добавьте другие скорости, если нужно
  }
  
  // Здесь можно добавить парсинг Swing, если он будет реализован

  return true; // Код успешно разобран
}

// --- КОНЕЦ ИЗМЕНЕНИЙ ---

// Тут должны быть все остальные методы из предыдущего ответа:
// setup(), dump_config(), traits(), control(), transmit_state_()

void ElectraACClimate::setup() {
  this->ac_.on();
  this->ac_.setFan(kElectraAcFanAuto);
  this->ac_.setMode(kElectraAcCool);
  this->ac_.setTemp(24);

  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
  } else {
    this->mode = climate::CLIMATE_MODE_COOL;
    this->target_temperature = 24;
    this->fan_mode = climate::CLIMATE_FAN_AUTO;
    this->swing_mode = climate::CLIMATE_SWING_OFF;
  }
}

void ElectraACClimate::dump_config() {
  ESP_LOGCONFIG(TAG, "Electra AC Climate:");
  LOG_CLIMATE("", "  ", this);
}

climate::ClimateTraits ElectraACClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(false);
  traits.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_FAN_ONLY,
      climate::CLIMATE_MODE_DRY,
      climate::CLIMATE_MODE_AUTO,
  });
  traits.set_supported_fan_modes({
      climate::CLIMATE_FAN_AUTO,
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
  });
  traits.set_supported_swing_modes({climate::CLIMATE_SWING_OFF});
  traits.set_visual_min_temperature(16.0f);
  traits.set_visual_max_temperature(30.0f);
  traits.set_visual_temperature_step(1.0f);
  return traits;
}

void ElectraACClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature().has_value())
    this->target_temperature = *call.get_target_temperature();
  if (call.get_fan_mode().has_value())
    this->fan_mode = *call.get_fan_mode();
  
  this->transmit_state_();
  this->publish_state();
}

void ElectraACClimate::transmit_state_() {
  if (this->mode == climate::CLIMATE_MODE_OFF) {
    this->ac_.off();
  } else {
    this->ac_.on();
    switch (this->mode) {
      case climate::CLIMATE_MODE_COOL:
        this->ac_.setMode(kElectraAcCool);
        break;
      case climate::CLIMATE_MODE_HEAT:
        this->ac_.setMode(kElectraAcHeat);
        break;
      case climate::CLIMATE_MODE_FAN_ONLY:
        this->ac_.setMode(kElectraAcFan);
        break;
      case climate::CLIMATE_MODE_DRY:
        this->ac_.setMode(kElectraAcDry);
        break;
      case climate::CLIMATE_MODE_AUTO:
        this->ac_.setMode(kElectraAcAuto);
        break;
      default:
        return;
    }
  }

  this->ac_.setTemp(static_cast<uint8_t>(this->target_temperature));

  switch (this->fan_mode) {
    case climate::CLIMATE_FAN_AUTO:
      this->ac_.setFan(kElectraAcFanAuto);
      break;
    case climate::CLIMATE_FAN_LOW:
      this->ac_.setFan(kElectraAcFanLow);
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      this->ac_.setFan(kElectraAcFanMedium);
      break;
    default:
        this->ac_.setFan(kElectraAcFanAuto);
  }

  ESP_LOGD(TAG, "Sending Electra AC state...");
  this->ac_.send(this->transmitter_);
}

}  // namespace electra_ac
}  // namespace esphome
