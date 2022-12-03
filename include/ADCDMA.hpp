#ifndef ADC_DMA_HPP
#define ADC_DMA_HPP

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

#define PICO_SAMP_FREQ 500000.f

template <uint32_t SIZE>
class DMAADC
{

  protected:

   /*!

  @param div - sample frequency divider, sample frequency = 500khz/div

  @param irq_on - specifie the number of samples which will fire request
  
  */
  void set_up_adc(float div,uint32_t irq_on)
  {
    // adc initialization
    adc_init();

    adc_fifo_setup(
      true,
      true,
      irq_on,
      false,
      false
    );

    // set sample frequency 500khz/div
    adc_set_clkdiv(div);

    // set adc pins
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);

    adc_set_round_robin(0b0000111);
  }

   /*!

  @param div - sample frequency divider, sample frequency = 500khz/div

  @param irq_on - specifie the number of samples which will fire request

  @param channel - a channel number
  
  */
  void set_up_adc(uint8_t channel,float div,uint32_t irq_on)
  {
    // adc initialization
    adc_init();

    // set adc pins
    adc_gpio_init(26+channel);

    adc_select_input(channel);

    adc_fifo_setup(
      true,
      true,
      irq_on,
      false,
      false
    );

    // set sample frequency 500khz/div
    adc_set_clkdiv(div);

    

  }

  dma_channel_config cfg;
  int32_t dma_channel;

  void set_up_dma()
  {
    dma_channel = dma_claim_unused_channel(true);

    cfg = dma_channel_get_default_config(dma_channel);

    // Reading from constant address, writing to incrementing byte addresses
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);

    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);
  }

  uint16_t buffer[SIZE];

  bool round;


  public:

  /*!

  @param div - sample frequency divider, sample frequency = 500khz/div

  @param irq_on - specifie the number of samples which will fire request

  */

  DMAADC(float div=0,uint32_t irq_on=1)
  { 
    set_up_adc(div,irq_on);

    set_up_dma();

    round=true;

  }

  /*!

  @param div - sample frequency divider, sample frequency = 500khz/div

  @param irq_on - specifie the number of samples which will fire request

  @param channel - a channel number
  
  */

  DMAADC(uint8_t channel,float div=0,uint32_t irq_on=1)
  { 
    set_up_adc(channel,div,irq_on);

    set_up_dma();

    round=false;

  }

  void set_divider(float div)
  {
    adc_set_clkdiv(div);
  }


  void set_freq(float freq)
  {
      float div=(PICO_SAMP_FREQ/freq)-1;

      set_divider(div);
  }
  
  void sample()
  {
    adc_fifo_drain();

    adc_run(false);

    dma_channel_configure(dma_channel,
    &cfg,
    buffer,
    &adc_hw->fifo,
    SIZE,
    true);

    if(round)
    {

    adc_select_input(0);
    }

    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_channel);
  }

  const uint16_t& operator[](uint32_t i)
  {
    if(i>=SIZE)
    {
      return buffer[0];
    }

    return buffer[i];
  }

  void copy(uint16_t *to,uint32_t amount=SIZE)
  {

    if(amount>SIZE)
    {
      amount=SIZE;
    }

    memcpy(to,buffer,amount*sizeof(uint8_t));

  }

  bool check_error()
  {
    return dma_channel==-1;
  }

  ~DMAADC()
  {

  }
};


#endif 