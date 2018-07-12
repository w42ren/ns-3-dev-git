  //  A simple UDP test
  //  Creates an adhoc wifi network with a number of STA = nsources + nsinks
  //  the sources send udp segments continously to the sinks
  //  the STA are positioned on a circle with radius 50
  //  the simulation runs for 10secs


  #include "ns3/core-module.h"
  #include "ns3/point-to-point-module.h"
  #include "ns3/network-module.h"
  #include "ns3/applications-module.h"
  #include "ns3/wifi-module.h"
  #include "ns3/mobility-module.h"
  #include "ns3/csma-module.h"
  #include "ns3/internet-module.h"
  #include "ns3/mobility-helper.h"
  #include "ns3/random-variable-stream-helper.h"
  #include <stdio.h>
  #include <stdlib.h>
  #include <time.h>

  using namespace ns3;

  int
  main (int argc, char *argv[])
  {
    uint32_t nsource = 3;// number of source stations
    uint32_t nsink = 3;// number of sink stations
    uint32_t i;// iteration index
    bool verbose = false;

    // Enable rts/cts all the time
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold",
    StringValue ("0"));

    // Create wifi nodes
    NodeContainer sta;
    sta.Create (nsource+nsink);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel (channel.Create ());

    // WifiHelper wifi = WifiHelper::Default ();
    WifiHelper wifi;
   if (verbose)
       {
         wifi.EnableLogComponents ();  // Turn on all Wifi logging
       }
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager"); //rate control algorithm
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);// IEEE 802.11g

    WifiMacHelper mac;

    // Set it to adhoc mode
    mac.SetType ("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install (phy, mac, sta);

    //Specify wifi modes to be stationary but randomly allocated within a disc
    MobilityHelper mobility;

    // Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
    mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                        "X", DoubleValue (0.0),
                                        "Y", DoubleValue (0.0),
                                        "rho", DoubleValue (50)
    );

    //mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator" ,
    //                            "Theta", var->GetValue (),
    //                            "Rho", var->GetValue (),
    //                             "X", DoubleValue (0.0),
    //                              "Y", DoubleValue (0.0)
    //                              );
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (sta);

    // Internet and Stack
    InternetStackHelper stack;
    stack.Install (sta);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipadd = address.Assign (devices);


    // Generate traffic

    // UDP: 10.1.1.1 ===>>> 10.1.1.2
    // Create one udpServer applications on 10.1.1.2
    uint16_t port_1 = 4000;
    UdpServerHelper server_1 (port_1);
    ApplicationContainer apps_1 = server_1.Install (sta.Get(1));
    apps_1.Start (Seconds (0.0));
    // Create one UdpClient application to send UDP datagrams from 10.1.1.1 to 10.1.1.2.
    uint32_t MaxPacketSize = 1024;
    Time interPacketInterval = Seconds (0.001);
    uint32_t maxPacketCount = 400000000;
    UdpClientHelper client_1 (ipadd.GetAddress (1), port_1);
    client_1.SetAttribute ("MaxPackets", UintegerValue
  (maxPacketCount));
    client_1.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client_1.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
    apps_1 = client_1.Install (sta.Get (0));
    apps_1.Start (Seconds (0.0));

    // UDP: 10.1.1.2 ===>>> 10.1.1.1
    // Create one udpServer applications on 10.1.1.1
    uint16_t port_2 = 2000;
    UdpServerHelper server_2 (port_2);
    ApplicationContainer apps_2 = server_2.Install (sta.Get(0));
    apps_2.Start (Seconds (0.0));
    // Create one UdpClient application to send UDP datagrams from 10.1.1.1 to 10.1.1.2.
    UdpClientHelper client_2 (ipadd.GetAddress (0), port_2);
    client_2.SetAttribute ("MaxPackets", UintegerValue
  (maxPacketCount));
    client_2.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client_2.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
    apps_2 = client_2.Install (sta.Get (1));
    apps_2.Start (Seconds (0.0));

    // Set routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // Set simulation stop time
    Simulator::Stop (Seconds (10.0));

    // Save pcap file for each station
    for (i=0; i<nsource+nsink; i+=2) {
            phy.EnablePcap ("src_ctstoself", devices.Get (i));
            phy.EnablePcap ("sink_ctstoself", devices.Get (i+1));
    }

    // Start the simulation
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;

 }
