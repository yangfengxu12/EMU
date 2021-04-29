

def LoRa_para_check( center_freq, tx_power, bandwidth, preamble_length, invert_IQ, sync_words, spread_factor, coding_rate, CRC, implicit_header, lowdatarateoptimize):

    # center_freq
    center_freq = 433e6
    print("Center frequency: 433 MHz")
    #tx power
    if tx_power > 14:
        printf("Tx power is " + tx_power + ", more than 14. Set it to 14")
        tx_power = 14
    elif tx_power < -4:
        printf("Tx power is " + tx_power + ", less than -4. Set it to -4")
        tx_power = -4
        
    bandwidth = 125
    preamble_length = 8
    invert_IQ = False
    sync_words = 1234
    # spread factor
    if spread_factor > 12:
        printf("SF is " + spread_factor + ", more than 12. Set it to 12")
        lora_spread_factor = 12
        
    elif lora_spread_factor < 7:
        printf("SF is " + spread_factor + ", less than 7. Set it to 7")
        lora_spread_factor = 7
        
    # coidng rate
    if coding_rate > 4:
        printf("coding_rate is " + coding_rate + ", more than 4. Set it to 4")
        coding_rate = 4
    elif coding_rate < 1:
        printf("coding_rate is " + coding_rate + ", less than 1. Set it to 1")
        coding_rate = 1

    # CRC
    if CRC != True or CRC != False:
        printf("CRC is " + CRC + ", is not True or False. Set it to False")
        CRC = False
        
    # implicit_header
    if implicit_header != True or implicit_header != False:
        printf("implicit_header is " + implicit_header + ", is not True or False. Set it to False")
        implicit_header = False

    
    if lowdatarateoptimize != True or lowdatarateoptimize != False:
        printf("lowdatarateoptimize is " + lowdatarateoptimize + ", is not True or False. Set it to False")
        lowdatarateoptimize = False

    print("center_freq:"+ center_freq)
    print("tx_power:"+ tx_power)
    print("bandwidth:"+ bandwidth)
    print("preamble_length:"+ preamble_length)
    print("invert_IQ:"+ invert_IQ)
    print("sync_words:"+ sync_words)

    print("spread_factor:"+ spread_factor)
    print("coding_rate:"+ coding_rate)
    print("CRC:"+ CRC)
    print("implicit_header:"+ implicit_header)
    print("lowdatarateoptimize:"+ lowdatarateoptimize)


