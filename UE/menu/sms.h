#ifndef SMS_H
#define SMS_H

#include "menu.h"
#include "ue.h"

class SMS : public Menu {
 private:
  UE& ue;
  MSISDN dst_msisdn;
  std::string sms_text;
 public:
  inline SMS(UE& ueParam, MSISDN msisdnParam, std::string textParam): ue(ueParam), dst_msisdn(std::move(msisdnParam)), sms_text(std::move(textParam)){}
  void execute() override;
};

#endif  // SMS_H