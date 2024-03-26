//
// Created by loumouli on 3/26/24.
//

#include "ft_nmap.h"

double update_srtt(double oldsrtt, double instanceRTT) {
    return oldsrtt + (instanceRTT - oldsrtt) / 8;
}

double update_rttvar(double oldsrtt, double oldrttvar, double instanceRTT) {
    return oldrttvar + (fabs(instanceRTT - oldsrtt) - oldsrtt) / 4;
}

double update_timeout(double srtt, double rttvar) {
    double result = srtt + rttvar * 4;
    if (result < 100)
        result = 100;
    else if (result > 10000)
        result = 10000;
    return result;
}

