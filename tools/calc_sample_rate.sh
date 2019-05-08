#!/bin/bash

# snip from "Section 17. 10-bit Analog-to-Digital Converter (ADC)"
# named: 61104E.pdf

# When the PBCLK is used as the conversion clock source, the ADRC bit = 0, the Tad is the period
# of the PBCLK after the prescaler ADCS<7:0> bits (AD1CON3<7:0>) are applied.
# The ADC has a maximum rate at which conversions may be completed. An Analog module clock,
# Tad , controls the conversion timing. The analog-to-digital conversion requires 12 clock
# periods (12 Tad ).
# The period of the ADC conversion clock is software selected using an 8-bit counter. There are
# 256 possible options for Tad , which are specified by the ADCS<7:0> bits (AD1CON3<7:0>).


# install sqlite3 or use mysql/etc. to run this or just see the results down below
cat << END | mysql -upaul -ppaul paul
-- "constants"
-- Fosc = 40,000,000 (system clock, from config bits) 60,000,000 datasheet examples
-- Fpb = 2  (peripheral bus clock: 2 system clocks, from config bits)

drop table if exists adc_matrix
;

create table adc_matrix
select abs(actualKHZ - khz)/khz as khz_error, (idealSampleTime - sample_time) as sample_error,
	FULL.*
from (
   select khz, perchanKHZ, 
	   1 / (ADCSvals * (cvt + samp) * ((1.0000 / Fosc) * pbclk)) as actualKHZ,
	   chans, samp,
	   ADCSvals, ADCSbits,
	   ((1.0000 / Fosc) * pbclk) as Tpb, 
	   (1.0000 / khz) / (cvt + samp) as Tad,
	   1.0000 / khz as idealSampleTime,
	   samp * ((1.0000 / khz) / (cvt + samp)) as sample_time,
	   ((1.0000 / khz) / (cvt + samp)) / ((1.0000 / Fosc) * pbclk) as ADC_clock_div
   from (
      select 2.0000 as pbclk -- peripheral bus clock
   ) PBCLK
   join (
      select (16.0000 * H.l + L.l) as samp
      from (
	 select 0 as l union select 1 as l union select 2 as l union select 3 as l union
	 select 4 as l union select 5 as l union select 6 as l union select 7 as l union
	 select 8 as l union select 9 as l union select 10 as l union select 11 as l union
	 select 12 as l union select 13 as l union select 14 as l union select 15 as l
      ) L
      join (
	 select 0 as l union select 1 as l union select 2 as l union select 3 as l union
	 select 4 as l union select 5 as l union select 6 as l union select 7 as l union
	 select 8 as l union select 9 as l union select 10 as l union select 11 as l union
	 select 12 as l union select 13 as l union select 14 as l union select 15 as l
      ) H
      having samp > 0 -- not allowed
      order by samp
   ) SAMPLETIME
   join (
      select 12.0000 as cvt -- according to the data sheet
   ) CONVERSIONTIME
   join (
      select 40000000.0000 as Fosc
      -- select 60000000.0000 as Fosc
   ) FOSC
   join (
      select chans * perchanKHZ as khz, perchanKHZ as perchanKHZ, chans
      from (
	 select 30.0000 as perchanKHZ union
	 select 60.0000 as perchanKHZ union
	 select 120.0000 as perchanKHZ union
	 select 150.0000 as perchanKHZ union
	 select 240.0000 as perchanKHZ union
	 select 500.0000 as perchanKHZ union
	 select 1000.0000 as perchanKHZ union
	 select 2000.0000 as perchanKHZ union
	 select 4000.0000 as perchanKHZ union
	 select 8000.0000 as perchanKHZ union
	 select 16000.0000 as perchanKHZ union
	 select 32000.0000 as perchanKHZ union
	 select 64000.0000 as perchanKHZ union
	 select 96000.0000 as perchanKHZ union
	 select 100000.0000 as perchanKHZ union
	 select 125000.0000 as perchanKHZ union
	 select 250000.0000 as perchanKHZ union
	 select 500000.0000 as perchanKHZ union
	 select 1000000.0000 as perchanKHZ
      ) PERCHANKHZ
      join (
	 select 1.0000 as chans union
	 select 2.0000 as chans union
	 select 4.0000 as chans
      ) CHANS
   ) KHZ
   join (
      select (16 * H.l + L.l) * 2 + 2 as ADCSvals, (16 * H.l + L.l) as ADCSbits
      from (
	 select 0 as l union select 1 as l union select 2 as l union select 3 as l union
	 select 4 as l union select 5 as l union select 6 as l union select 7 as l union
	 select 8 as l union select 9 as l union select 10 as l union select 11 as l union
	 select 12 as l union select 13 as l union select 14 as l union select 15 as l
      ) L
      join (
	 select 0 as l union select 1 as l union select 2 as l union select 3 as l union
	 select 4 as l union select 5 as l union select 6 as l union select 7 as l union
	 select 8 as l union select 9 as l union select 10 as l union select 11 as l union
	 select 12 as l union select 13 as l union select 14 as l union select 15 as l
      ) H
      order by ADCSvals
   ) ADCS
) FULL
order by khz, khz_error, sample_error, perchanKHZ, chans, Tad, actualKHZ
;

select count(1) from adc_matrix -- |  2937600 |
;

-- add a unique integer id to each column
alter table adc_matrix add column id int auto_increment, add index k_id (id)
;

-- select MINS.*, ADCM.*
-- from adc_matrix ADCM
-- inner join ( 
--    select khz, min(khz_error) as min_khz_error, min(id) as min_id
--    from adc_matrix
--    group by khz, chans
-- ) MINS on ADCM.id = MINS.min_id
-- where ADCM.khz <= 1000000 and ADCM.actualKHZ <= 1000000
-- order by perchanKHZ, chans
-- ;

select ADCM.khz, actualKHZ, perchanKHZ, chans, samp, ADCSbits -- , min_khz_error
from adc_matrix ADCM
inner join ( 
   select khz, min(chans) as min_chans, min(khz_error) as min_khz_error, min(id) as min_id
   from adc_matrix
   group by khz, chans
) MINS on ADCM.id = MINS.min_id
where ADCM.khz <= 1000000 and ADCM.actualKHZ <= 1000000
order by perchanKHZ, chans
;

-- +------------------+-------------+--------------+--------+----------+----------+
-- | khz              | actualKHZ   | perchanKHZ   | chans  | samp     | ADCSbits |
-- +------------------+-------------+--------------+--------+----------+----------+
-- |      30.00000000 |    146.3015 |      30.0000 | 1.0000 | 255.0000 |      255 |
-- |      60.00000000 |    146.3015 |      30.0000 | 2.0000 | 255.0000 |      255 |
-- |     120.00000000 |    146.3015 |      30.0000 | 4.0000 | 255.0000 |      255 |
-- |      60.00000000 |    146.3015 |      60.0000 | 1.0000 | 255.0000 |      255 |
-- |     120.00000000 |    146.3015 |      60.0000 | 2.0000 | 255.0000 |      255 |
-- |     240.00000000 |    240.0038 |      60.0000 | 4.0000 | 239.0000 |      165 |
-- |     120.00000000 |    146.3015 |     120.0000 | 1.0000 | 255.0000 |      255 |
-- |     240.00000000 |    240.0038 |     120.0000 | 2.0000 | 239.0000 |      165 |
-- |     480.00000000 |    480.0077 |     120.0000 | 4.0000 | 239.0000 |       82 |
-- |     150.00000000 |    149.8127 |     150.0000 | 1.0000 | 255.0000 |      249 |
-- |     300.00000000 |    300.0300 |     150.0000 | 2.0000 | 190.0000 |      164 |
-- |     600.00000000 |    600.0600 |     150.0000 | 4.0000 | 153.0000 |      100 |
-- |     240.00000000 |    240.0038 |     240.0000 | 1.0000 | 239.0000 |      165 |
-- |     480.00000000 |    480.0077 |     240.0000 | 2.0000 | 239.0000 |       82 |
-- |     960.00000000 |    960.0614 |     240.0000 | 4.0000 | 236.0000 |       41 |
-- |     500.00000000 |    500.0000 |     500.0000 | 1.0000 | 238.0000 |       79 |
-- |    1000.00000000 |   1000.0000 |     500.0000 | 2.0000 | 238.0000 |       39 |
-- |    2000.00000000 |   2000.0000 |     500.0000 | 4.0000 | 238.0000 |       19 |
-- |    1000.00000000 |   1000.0000 |    1000.0000 | 1.0000 | 238.0000 |       39 |
-- |    2000.00000000 |   2000.0000 |    1000.0000 | 2.0000 | 238.0000 |       19 |
-- |    4000.00000000 |   4000.0000 |    1000.0000 | 4.0000 | 238.0000 |        9 |
-- |    2000.00000000 |   2000.0000 |    2000.0000 | 1.0000 | 238.0000 |       19 |
-- |    4000.00000000 |   4000.0000 |    2000.0000 | 2.0000 | 238.0000 |        9 |
-- |    8000.00000000 |   8000.0000 |    2000.0000 | 4.0000 | 238.0000 |        4 |
-- |    4000.00000000 |   4000.0000 |    4000.0000 | 1.0000 | 238.0000 |        9 |
-- |    8000.00000000 |   8000.0000 |    4000.0000 | 2.0000 | 238.0000 |        4 |
-- |   16000.00000000 |  16000.0000 |    4000.0000 | 4.0000 | 113.0000 |        4 |
-- |    8000.00000000 |   8000.0000 |    8000.0000 | 1.0000 | 238.0000 |        4 |
-- |   16000.00000000 |  16000.0000 |    8000.0000 | 2.0000 | 113.0000 |        4 |
-- |   32000.00000000 |  32051.2821 |    8000.0000 | 4.0000 | 144.0000 |        1 |
-- |   16000.00000000 |  16000.0000 |   16000.0000 | 1.0000 | 113.0000 |        4 |
-- |   32000.00000000 |  32051.2821 |   16000.0000 | 2.0000 | 144.0000 |        1 |
-- |   64000.00000000 |  64102.5641 |   16000.0000 | 4.0000 | 144.0000 |        0 |
-- |   32000.00000000 |  32051.2821 |   32000.0000 | 1.0000 | 144.0000 |        1 |
-- |   64000.00000000 |  64102.5641 |   32000.0000 | 2.0000 | 144.0000 |        0 |
-- |  128000.00000000 | 128205.1282 |   32000.0000 | 4.0000 |  66.0000 |        0 |
-- |   64000.00000000 |  64102.5641 |   64000.0000 | 1.0000 | 144.0000 |        0 |
-- |  128000.00000000 | 128205.1282 |   64000.0000 | 2.0000 |  66.0000 |        0 |
-- |  256000.00000000 | 256410.2564 |   64000.0000 | 4.0000 |  27.0000 |        0 |
-- |   96000.00000000 |  96153.8462 |   96000.0000 | 1.0000 |  92.0000 |        0 |
-- |  192000.00000000 | 192307.6923 |   96000.0000 | 2.0000 |  40.0000 |        0 |
-- |  384000.00000000 | 384615.3846 |   96000.0000 | 4.0000 |  14.0000 |        0 |
-- |  100000.00000000 | 100000.0000 |  100000.0000 | 1.0000 |  88.0000 |        0 |
-- |  200000.00000000 | 200000.0000 |  100000.0000 | 2.0000 |  38.0000 |        0 |
-- |  400000.00000000 | 400000.0000 |  100000.0000 | 4.0000 |  13.0000 |        0 |
-- |  125000.00000000 | 125000.0000 |  125000.0000 | 1.0000 |  68.0000 |        0 |
-- |  250000.00000000 | 250000.0000 |  125000.0000 | 2.0000 |  28.0000 |        0 |
-- |  500000.00000000 | 500000.0000 |  125000.0000 | 4.0000 |   8.0000 |        0 |
-- |  250000.00000000 | 250000.0000 |  250000.0000 | 1.0000 |  28.0000 |        0 |
-- |  500000.00000000 | 500000.0000 |  250000.0000 | 2.0000 |   8.0000 |        0 |
-- | 1000000.00000000 | 769230.7692 |  250000.0000 | 4.0000 |   1.0000 |        0 |
-- |  500000.00000000 | 500000.0000 |  500000.0000 | 1.0000 |   8.0000 |        0 |
-- | 1000000.00000000 | 769230.7692 |  500000.0000 | 2.0000 |   1.0000 |        0 |
-- | 1000000.00000000 | 769230.7692 | 1000000.0000 | 1.0000 |   1.0000 |        0 |
-- +------------------+-------------+--------------+--------+----------+----------+

select perchanKHZ, chans, samp, ADCSbits -- , min_khz_error
from adc_matrix ADCM
inner join ( 
   select khz, min(chans) as min_chans, min(khz_error) as min_khz_error, min(id) as min_id
   from adc_matrix
   group by khz, chans
) MINS on ADCM.id = MINS.min_id
where ADCM.khz <= 1000000 and ADCM.actualKHZ <= 1000000
order by perchanKHZ, chans
;
-- +--------------+--------+----------+----------+
-- | perchanKHZ   | chans  | samp     | ADCSbits |
-- +--------------+--------+----------+----------+
-- |      30.0000 | 1.0000 | 255.0000 |      255 |
-- |      30.0000 | 2.0000 | 255.0000 |      255 |
-- |      30.0000 | 4.0000 | 255.0000 |      255 |
-- |      60.0000 | 1.0000 | 255.0000 |      255 |
-- |      60.0000 | 2.0000 | 255.0000 |      255 |
-- |      60.0000 | 4.0000 | 239.0000 |      165 |
-- |     120.0000 | 1.0000 | 255.0000 |      255 |
-- |     120.0000 | 2.0000 | 239.0000 |      165 |
-- |     120.0000 | 4.0000 | 239.0000 |       82 |
-- |     150.0000 | 1.0000 | 255.0000 |      249 |
-- |     150.0000 | 2.0000 | 190.0000 |      164 |
-- |     150.0000 | 4.0000 | 153.0000 |      100 |
-- |     240.0000 | 1.0000 | 239.0000 |      165 |
-- |     240.0000 | 2.0000 | 239.0000 |       82 |
-- |     240.0000 | 4.0000 | 236.0000 |       41 |
-- |     500.0000 | 1.0000 | 238.0000 |       79 |
-- |     500.0000 | 2.0000 | 238.0000 |       39 |
-- |     500.0000 | 4.0000 | 238.0000 |       19 |
-- |    1000.0000 | 1.0000 | 238.0000 |       39 |
-- |    1000.0000 | 2.0000 | 238.0000 |       19 |
-- |    1000.0000 | 4.0000 | 238.0000 |        9 |
-- |    2000.0000 | 1.0000 | 238.0000 |       19 |
-- |    2000.0000 | 2.0000 | 238.0000 |        9 |
-- |    2000.0000 | 4.0000 | 238.0000 |        4 |
-- |    4000.0000 | 1.0000 | 238.0000 |        9 |
-- |    4000.0000 | 2.0000 | 238.0000 |        4 |
-- |    4000.0000 | 4.0000 | 113.0000 |        4 |
-- |    8000.0000 | 1.0000 | 238.0000 |        4 |
-- |    8000.0000 | 2.0000 | 113.0000 |        4 |
-- |    8000.0000 | 4.0000 | 144.0000 |        1 |
-- |   16000.0000 | 1.0000 | 113.0000 |        4 |
-- |   16000.0000 | 2.0000 | 144.0000 |        1 |
-- |   16000.0000 | 4.0000 | 144.0000 |        0 |
-- |   32000.0000 | 1.0000 | 144.0000 |        1 |
-- |   32000.0000 | 2.0000 | 144.0000 |        0 |
-- |   32000.0000 | 4.0000 |  66.0000 |        0 |
-- |   64000.0000 | 1.0000 | 144.0000 |        0 |
-- |   64000.0000 | 2.0000 |  66.0000 |        0 |
-- |   64000.0000 | 4.0000 |  27.0000 |        0 |
-- |   96000.0000 | 1.0000 |  92.0000 |        0 |
-- |   96000.0000 | 2.0000 |  40.0000 |        0 |
-- |   96000.0000 | 4.0000 |  14.0000 |        0 |
-- |  100000.0000 | 1.0000 |  88.0000 |        0 |
-- |  100000.0000 | 2.0000 |  38.0000 |        0 |
-- |  100000.0000 | 4.0000 |  13.0000 |        0 |
-- |  125000.0000 | 1.0000 |  68.0000 |        0 |
-- |  125000.0000 | 2.0000 |  28.0000 |        0 |
-- |  125000.0000 | 4.0000 |   8.0000 |        0 |
-- |  250000.0000 | 1.0000 |  28.0000 |        0 |
-- |  250000.0000 | 2.0000 |   8.0000 |        0 |
-- |  250000.0000 | 4.0000 |   1.0000 |        0 |
-- |  500000.0000 | 1.0000 |   8.0000 |        0 |
-- |  500000.0000 | 2.0000 |   1.0000 |        0 |
-- | 1000000.0000 | 1.0000 |   1.0000 |        0 |
-- +--------------+--------+----------+----------+
END
