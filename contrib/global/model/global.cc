/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "global.h"

namespace ns3 {

    AsciiTraceHelper asciiTraceHelperInterference;
    Ptr<OutputStreamWrapper> interference_stream = asciiTraceHelperInterference.CreateFileStream("/home/natasha/repos/ns-3-dev-git/SignalNoiseInterference_5G.csv");
}

