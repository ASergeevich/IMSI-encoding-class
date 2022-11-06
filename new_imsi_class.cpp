#include <iostream>
#include <cstdint>


class IMSI
{
    enum Separator
    {
        MCC,
        MNC,
        MSIN,
        LENGTH_MSIN = 10,
        LENGTH_THE_AMERICAN_MNC = 3,
        LENGTH_THE_EUROPEAN_MNC = 2,
    };

    union Union
    {
        std::uint64_t number;
        std::uint16_t the_first_components[2];
        std::uint32_t the_second_components[2];
    };

    Union components_of_imsi{0};

    void decoding(std::uint64_t bdc_imsi)
    {
        std::uint64_t imsi{0};

        for(std::uint64_t decimal_place{1}; bdc_imsi; decimal_place *= 10)
        {
            std::uint16_t low_mask {0xF};
            low_mask &= bdc_imsi;
            bdc_imsi >>= 4;
            imsi += (low_mask * decimal_place);
        }

        std::uint16_t imsi_length{0}, separator_length{LENGTH_MSIN}, length{1};

        for(std::uint64_t iteration{imsi}; iteration; iteration /= 10)
        {
            ++imsi_length;
        }


        Separator current_flag{MSIN};
        std::uint64_t time_number{0}, decimal_place{1};

        auto reset_variables
        {
            [&time_number, &length, &current_flag, &decimal_place](Separator flag){

                current_flag = flag;
                time_number = 0;
                length = 1;
                decimal_place = 1;
            }
        };

        for( ; current_flag != MCC; imsi /= 10)
        {
            std::uint16_t num = imsi % 10;
            time_number += (decimal_place * num);

            if(current_flag == MSIN && length == separator_length)
            {
                components_of_imsi.the_second_components[1] |= time_number;
                time_number >>= 32;
                components_of_imsi.the_first_components[1] |= time_number;
                components_of_imsi.the_first_components[1] <<= 14;

                reset_variables(MNC);

                if(imsi_length == 15)
                {
                    separator_length = LENGTH_THE_EUROPEAN_MNC;
                }

                else
                {
                    separator_length = LENGTH_THE_AMERICAN_MNC;
                }
                continue;
            }

            else if(current_flag == MNC && length == separator_length)
            {
                components_of_imsi.the_first_components[1] |= time_number;

                reset_variables(MCC);
                continue;
            }

            decimal_place *= 10;
            ++length;
        }

        components_of_imsi.the_first_components[0] = static_cast<std::uint16_t>(imsi);
    }

    public:
    IMSI(std::uint64_t bdc_imsi)
    {
        decoding(bdc_imsi);
    }

    std::uint16_t get_mcc() const { return components_of_imsi.the_first_components[0]; }

    std::uint16_t get_mnc() const
    {
        std::uint16_t mnc{0xFFF};
        mnc &= components_of_imsi.the_first_components[1];

        return mnc;
    }

    std::uint64_t get_msin() const
    {
        std::uint16_t time_number{components_of_imsi.the_first_components[1]};
        std::uint64_t msin{0};
        time_number >>= 14;
        msin |= time_number;
        msin <<= 32;
        msin |= components_of_imsi.the_second_components[1];

        return msin;
    }

};
