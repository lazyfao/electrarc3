#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/remote_receiver/remote_receiver.h" // <-- Добавили

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Electra.h>

namespace esphome {
namespace electra_ac {

class ElectraACClimate : public climate::Climate, public Component {
 public:
  void setup() override;
  void dump_config() override;

  void set_transmitter(climate::remote_base::RemoteTransmitterBase *transmitter) {
    this->transmitter_ = transmitter->get_ir_sender();
  }
  
  // --- НАЧАЛО ИЗМЕНЕНИЙ ---
  void set_receiver(remote_receiver::RemoteReceiverComponent *receiver);
  // --- КОНЕЦ ИЗМЕНЕНИЙ ---

 protected:
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;
  
  void transmit_state_();
  // --- НАЧАЛО ИЗМЕНЕНИЙ ---
  bool parse_state_(const remote_receiver::RemoteReceiveData &data);
  // --- КОНЕЦ ИЗМЕНЕНИЙ ---

  IRElectraAc ac_{kIrLed};
  IRsend *transmitter_{nullptr};
};

}  // namespace electra_ac
}  // namespace esphome
