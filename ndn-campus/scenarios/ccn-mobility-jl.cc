/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Standard C++ modules
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iterator>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <vector>

// Random modules
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/variate_generator.hpp>

// ns3 modules
#include <ns3-dev/ns3/applications-module.h>
#include <ns3-dev/ns3/bridge-helper.h>
#include <ns3-dev/ns3/csma-module.h>
#include <ns3-dev/ns3/core-module.h>
#include <ns3-dev/ns3/mobility-module.h>
#include <ns3-dev/ns3/network-module.h>
#include <ns3-dev/ns3/point-to-point-module.h>
#include <ns3-dev/ns3/wifi-module.h>

// ndnSIM modules
#include <ns3-dev/ns3/ndnSIM-module.h>
#include <ns3-dev/ns3/ndnSIM/utils/tracers/ipv4-rate-l3-tracer.h>
#include <ns3-dev/ns3/ndnSIM/utils/tracers/ipv4-seqs-app-tracer.h>

using namespace ns3;
using namespace boost;

namespace br = boost::random;

typedef struct timeval TIMER_TYPE;
#define TIMER_NOW(_t) gettimeofday (&_t,NULL);
#define TIMER_SECONDS(_t) ((double)(_t).tv_sec + (_t).tv_usec*1e-6)
#define TIMER_DIFF(_t1, _t2) (TIMER_SECONDS (_t1)-TIMER_SECONDS (_t2))

char scenario[250] = "CCNWireless";

NS_LOG_COMPONENT_DEFINE (scenario);

// Number generator
br::mt19937_64 gen;

// Obtains a random number from a uniform distribution between min and max.
// Must seed number generator to ensure randomness at runtime.
int obtain_Num(int min, int max) {
    br::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

std::vector<Ptr<Node> > getVector(NodeContainer node) {

	uint32_t size = node.GetN ();

	std::vector<Ptr<Node> > nodemutable;

	// Copy the Node pointers into a mutable vector
	for (uint32_t i = 0; i < size; i++) {
		nodemutable.push_back (node.Get(i));
	}

	NS_LOG_INFO ("getVector: returning Node vector");

	return nodemutable;
}

// Randomly picks toAsig nodes from a vector that has nodesAvailable in size
std::vector<Ptr<Node> > assignNodes(std::vector<Ptr<Node> > nodes, int toAsig, int nodesAvailable) {

	char buffer[250];

	sprintf(buffer, "assignNodes: to assign %d, left %d", toAsig, nodesAvailable);

	NS_LOG_INFO (buffer);

	std::vector<Ptr<Node> > assignedNodes;

	uint32_t assignMin = nodesAvailable - toAsig;

	// Apply Fisher-Yates shuffle
	for (uint32_t i = nodesAvailable; i > assignMin; i--)
	{
		// Get a random number
		int toSwap = obtain_Num (0, i);
		// Push into the client container
		assignedNodes.push_back (nodes[toSwap]);
		// Swap the obtained number with the last element
		std::swap (nodes[toSwap], nodes[i]);
	}

	return assignedNodes;
}

// Obtains a random list of num_clients clients and num_servers servers from a NodeContainer
tuple<std::vector<Ptr<Node> >, std::vector<Ptr<Node> > > assignClientsandServers(NodeContainer nodes, int num_clients, int num_servers) {

	char buffer[250];

	// Get the number of nodes in the simulation
	uint32_t size = nodes.GetN ();

	sprintf(buffer, "assignClientsandServers, we have %d nodes, will assign %d clients and %d servers", size, num_clients, num_servers);

	NS_LOG_INFO (buffer);

	// Check that we haven't asked for a scenario where we don't have enough Nodes to fulfill
	// the requirements
	if (num_clients + num_servers > size) {
		NS_LOG_INFO("assignClientsandServer, required number bigger than container size!");
		return tuple<std::vector<Ptr<Node> >, std::vector<Ptr<Node> > > ();
	}

	std::vector<Ptr<Node> > nodemutable = getVector(nodes);

	std::vector<Ptr<Node> > ClientContainer = assignNodes(nodemutable, num_clients, size-1);

	std::vector<Ptr<Node> > ServerContainer = assignNodes(nodemutable, num_servers, size-1-num_clients);

	return tuple<std::vector<Ptr<Node> >, std::vector<Ptr<Node> > > (ClientContainer, ServerContainer);
}

// Returns a randomly picked num of Nodes from nodes Container
std::vector<Ptr<Node> > assignWithinContainer (NodeContainer nodes, int num)
{
	char buffer[250];

	// Get the number of nodes in the simulation
	uint32_t size = nodes.GetN ();

	sprintf(buffer, "assignWithinContainer, we have %d nodes, will assign %d", size, num);

	NS_LOG_INFO (buffer);

	if (num > size) {
		NS_LOG_INFO("assignWithinContainer, required number bigger than container size!");
		return std::vector<Ptr<Node> >();
	}

	std::vector<Ptr<Node> > nodemutable = getVector(nodes);

	return assignNodes(nodemutable, num, size-1);

}

// Function to get a complete Random setup
tuple<std::vector<Ptr<Node> >, std::vector<Ptr<Node> > > assignCompleteRandom(int num_clients, int num_servers) {

	// Obtain all the node used in the simulation
	NodeContainer global = NodeContainer::GetGlobal ();

	return assignClientsandServers(global, num_clients, num_servers);
}

int main (int argc, char *argv[])
{
	// These are our scenario arguments
	uint32_t contentsize = 10485760;	// Size in bytes of the content to transfer
	uint32_t aps = 6;					// Number of clients in the network
	uint32_t mobile = 1;				// Number of mobile terminals
	uint32_t clients = 1;				// Number of clients in the network
	uint32_t nodes = 12;				// Number of nodes in the network
	double sec = 0.0;					// Movement start
	double waitint = 0.6;				// Wait at AP
	double travelTime = 3.0;			// Travel time within APs

	char results[250] = "results";
	char buffer[250];

	CommandLine cmd;

	cmd.AddValue ("aps", "Number of APs [5]", aps);
	cmd.AddValue ("mobile", "Number of mobile terminals", mobile);
	cmd.AddValue ("clients", "Number of clients", clients);
	cmd.AddValue ("network", "Number of network nodes", nodes);
	cmd.AddValue ("contentsize",
			"Total number of bytes for application to send", contentsize);
	cmd.AddValue ("results", "Directory to place results", results);
	cmd.AddValue ("start", "Starting second", sec);
	cmd.AddValue ("waitint", "Wait interval between APs", waitint);
	cmd.AddValue ("travel", "Travel time between APs", travelTime);
	cmd.Parse (argc,argv);

	// Node definitions for mobile terminals
	NodeContainer mobileTerminalContainer;
	mobileTerminalContainer.Create(mobile);

	// Nodes for APs
	NodeContainer apsContainer;
	apsContainer.Create (aps);

	// LAN nodes
	NodeContainer networkNodes;
	networkNodes.Create (nodes);

	// First level nodes (after APs)
	NodeContainer middleNodes;
	middleNodes.Create (aps);

	int desiredConnections = 2;
	int lvl3nodes = aps / desiredConnections;

	int lansize = nodes / lvl3nodes;

	// Second level nodes, after first level
	NodeContainer lanrouterNodes;
	lanrouterNodes.Create (lvl3nodes);

	// Container for all routers
	NodeContainer allRouters;
	allRouters.Add (apsContainer);
	allRouters.Add (middleNodes);
	allRouters.Add (lanrouterNodes);

	// Container for all user nodes
	NodeContainer allUserNodes;
	allUserNodes.Add (mobileTerminalContainer);
	allUserNodes.Add (networkNodes);

	// Make sure to seed our random
	gen.seed(std::time(0));

	// With the network assigned, time to randomly obtain clients and servers
	NS_LOG_INFO ("Obtaining the clients and servers");

	// Obtain the random lists of server and clients
	std::vector<Ptr<Node> > clientVector = assignWithinContainer(networkNodes, clients);

	NodeContainer clientNodes;
	std::vector<uint32_t> clientNodeIds;

	// We have to manually introduce the Ptr<Node> to the NodeContainers
	// We do this to make them easier to control later
	for (uint32_t i = 0; i < clients ; i++)
	{
		Ptr<Node> tmp = clientVector[i];

		uint32_t nodeNum = tmp->GetId();

		sprintf (buffer, "Adding client node: %d", nodeNum);
		NS_LOG_INFO (buffer);

		clientNodes.Add(tmp);
		clientNodeIds.push_back(nodeNum);
	}

	NS_LOG_INFO ("Placing APs");

	// Mobility definition for APs
	MobilityHelper mobilityStations;

	mobilityStations.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (0.0),
			"MinY", DoubleValue (0.0),
			"DeltaX", DoubleValue (30.0),
			"DeltaY", DoubleValue (0.0),
			"GridWidth", UintegerValue (apsContainer.GetN ()),
			"LayoutType", StringValue ("RowFirst"));
	mobilityStations.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	mobilityStations.Install (apsContainer);

	// Mobility definition for Level 1
	MobilityHelper mobilitylvl1;

	mobilitylvl1.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (0.0),
			"MinY", DoubleValue (20.0),
			"DeltaX", DoubleValue (30.0),
			"DeltaY", DoubleValue (0.0),
			"GridWidth", UintegerValue (middleNodes.GetN ()),
			"LayoutType", StringValue ("RowFirst"));
	mobilitylvl1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	mobilitylvl1.Install (middleNodes);

	// Mobility definition for Level 2
	MobilityHelper mobilitylvl2;

	mobilitylvl2.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (15.0),
			"MinY", DoubleValue (40.0),
			"DeltaX", DoubleValue (60.0),
			"DeltaY", DoubleValue (0.0),
			"GridWidth", UintegerValue (lanrouterNodes.GetN ()),
			"LayoutType", StringValue ("RowFirst"));
	mobilitylvl2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	mobilitylvl2.Install (lanrouterNodes);

	NS_LOG_INFO ("Placing mobile terminals");

	MobilityHelper mobilityTerminals;

	Vector diff = Vector(0.0, -8.0, 0.0);

	Vector pos;

	Ptr<ListPositionAllocator> initialAlloc = CreateObject<ListPositionAllocator> ();

	Ptr<MobilityModel> mob = apsContainer.Get (0)->GetObject<MobilityModel>();

	pos = mob->GetPosition();

	initialAlloc->Add (pos + diff);

	// Put everybody into a line
	mobilityTerminals.SetPositionAllocator(initialAlloc);
	mobilityTerminals.SetMobilityModel("ns3::WaypointMobilityModel");
	mobilityTerminals.Install(mobileTerminalContainer.Get (0));

	Ptr<WaypointMobilityModel> staWaypointMobility = DynamicCast<WaypointMobilityModel>(mobileTerminalContainer.Get (0)->GetObject<MobilityModel> ());

	sprintf(buffer, "Assigning waypoints - start: %f, pause: %f, travel: %f", sec, waitint, travelTime);

	NS_LOG_INFO (buffer);

	for (int j = 0; j < aps; j++)
	{
		mob = apsContainer.Get (j)->GetObject<MobilityModel>();

		Vector wayP = mob->GetPosition () + diff;

		staWaypointMobility->AddWaypoint (Waypoint(Seconds(sec), wayP));
		staWaypointMobility->AddWaypoint (Waypoint(Seconds(sec + waitint), wayP));

		sec += waitint + travelTime;
	}

	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
			"DataMode", StringValue ("OfdmRate24Mbps"));

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel");
	wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");

	//YansWifiPhy wifiPhy = YansWifiPhy::Default();
	YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default ();
	wifiPhyHelper.SetChannel (wifiChannel.Create ());
	wifiPhyHelper.Set("TxPowerStart", DoubleValue(5));
	wifiPhyHelper.Set("TxPowerEnd", DoubleValue(5));

	NqosWifiMacHelper wifiMacHelper = NqosWifiMacHelper::Default ();
	wifiMacHelper.SetType("ns3::AdhocWifiMac");

	NetDeviceContainer wifiAPNetDevices = wifi.Install (wifiPhyHelper, wifiMacHelper, apsContainer);

	NetDeviceContainer wifiMTNetDevices = wifi.Install (wifiPhyHelper, wifiMacHelper, mobileTerminalContainer);

	NetDeviceContainer p2pAPMiddleDevices;

	// Connect APs to first level cache with p2p
	PointToPointHelper p2p_1gb5ms;
	p2p_1gb5ms.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
	p2p_1gb5ms.SetChannelAttribute ("Delay", StringValue ("5ms"));

	for (int j = 0; j < aps; j++)
	{
		p2pAPMiddleDevices.Add (
				p2p_1gb5ms.Install (apsContainer.Get (j),
						middleNodes.Get (j)));
	}

	// Connect first level nodes with second level nodes with p2p
	NetDeviceContainer p2pMiddleLanRouterDevices;

	int step = 0;

	for (int j = 0; j < lvl3nodes; j++)
	{
		for (int k = 0; k < desiredConnections; k++)
		{
			p2pMiddleLanRouterDevices.Add (
					p2p_1gb5ms.Install(middleNodes.Get (k + step),
							lanrouterNodes.Get (j)));
		}

		step += desiredConnections;
	}

	std::vector<NodeContainer> lans;

	step = 0;

	for (int i = 0; i < lvl3nodes; i++)
	{
		NodeContainer tmp;
		tmp.Add (lanrouterNodes.Get (i));

		for (int k = 0; k < lansize; k++)
		{
			tmp.Add (networkNodes.Get (k + step));
		}

		step += lansize;
		lans.push_back (tmp);
	}

	std::vector<CsmaHelper> csmaV;

	for (int j = 0; j < lvl3nodes; j++)
	{
		CsmaHelper csma;
		csma.SetChannelAttribute("Delay", StringValue("2ms"));
		csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));

		csmaV.push_back (csma);
	}
	CsmaHelper csma;

	std::vector<NetDeviceContainer> lanDevices;

	for (int j = 0; j < lvl3nodes; j++)
	{
		lanDevices.push_back (csmaV[j].Install (lans[j]));
	}

	// Now install content stores and the rest on the middle node. Leave
	// out clients and the mobile node
	NS_LOG_INFO ("Installing NDN stack on routers");
	ndn::StackHelper ndnHelperRouters;
	ndnHelperRouters.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
	ndnHelperRouters.SetContentStore ("ns3::ndn::cs::Freshness::Lru", "MaxSize", "1000");
	ndnHelperRouters.SetDefaultRoutes (true);
	ndnHelperRouters.Install (allRouters);

	ndn::StackHelper ndnHelperUsers;
	ndnHelperUsers.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
	ndnHelperUsers.SetContentStore ("ns3::ndn::cs::Nocache");
	ndnHelperUsers.SetDefaultRoutes (true);
	ndnHelperUsers.Install (allUserNodes);

	NS_LOG_INFO ("Installing Producer Application");
	// Create the producer on the mobile node
	ndn::AppHelper producerHelper ("ns3::ndn::Producer");
	producerHelper.SetPrefix ("/waseda/sato");
	producerHelper.SetAttribute ("PayloadSize", UintegerValue (contentsize));
	producerHelper.Install (mobileTerminalContainer);

	NS_LOG_INFO ("Installing Consumer Application");
	// Create the consumer on the randomly selected node
	ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
	consumerHelper.SetPrefix ("/waseda/sato");
	consumerHelper.SetAttribute ("Frequency", DoubleValue (10.0));
	consumerHelper.SetAttribute ("MaxSeq", IntegerValue  (100));
	consumerHelper.Install (clientNodes);

	// Filename
	char filename[250];

	// File ID
	char fileId[250];

	// Create the file identifier
	sprintf(fileId, "%02d-%03d-%03-%*d.txt", mobile, clients, nodes, 12, contentsize);

	// Print server nodes to file
	sprintf(filename, "%s/%s-servers-%d", results, scenario, fileId);

	/*NS_LOG_INFO ("Printing node files");
	std::ofstream serverFile;
	serverFile.open (filename);
	for (int i = 0; i < serverNodeIds.size(); i++) {
		serverFile << serverNodeIds[i] << std::endl;
	}
	serverFile.close();*/

	/*sprintf(filename, "%s/%s-clients-%s", results, scenario, fileId);

	std::ofstream clientFile;
	clientFile.open (filename);
	for (int i = 0; i < clientNodeIds.size(); i++) {
		clientFile << clientNodeIds[i] << std::endl;
	}
	clientFile.close();*/

	/*NS_LOG_INFO ("Installing tracers");
	sprintf (filename, "%s/%s-aggregate-trace-%s", results, scenario, fileId);
	ndn::L3AggregateTracer::InstallAll(filename, Seconds (1.0));

	sprintf (filename, "%s/%s-rate-trace-%s", results, scenario, fileId);
	ndn::L3RateTracer::InstallAll (filename, Seconds (1.0));

	sprintf (filename, "%s/%s-app-delays-%s", results, scenario, fileId);
	ndn::AppDelayTracer::InstallAll (filename);

	sprintf (filename, "%s/%s-drop-trace-%s", results, scenario, fileId);
	L2RateTracer::InstallAll (filename, Seconds (0.5));

	sprintf (filename, "%s/%s-cs-trace-%s", results, scenario, fileId);
	ndn::CsTracer::InstallAll (filename, Seconds (1));*/

	NS_LOG_INFO ("Ready for execution!");
	Simulator::Stop (Seconds (40.0));
	Simulator::Run ();
	Simulator::Destroy ();
}
