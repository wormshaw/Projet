#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/animation-interface.h"
#include "ns3/attribute-helper.h"
#include "ns3/node.h"
#include "ns3/energy-module.h"
#include "ns3/rv-battery-model.h"
#include "ns3/wifi-radio-energy-model-helper.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>


#define NUM_NODES 100
#define CLUSTER_PERCENTAGE 0.2 //Cluster percent
#define TOT_ROUNDS 200
#define INIT_ENERGY 100
#define SEUIL 80 //seuil de pourcentage d'énergie a partir duquel le reseau est considere comme faible
//on le met a 80 pour reduire le temps de compilation

using namespace ns3;
using namespace std;

class noeud : public ns3::Node
{

    using Node::Node;

    public:

        static TypeId GetTypeId (void);

        double r_randomnumber;
        vector<vector<double>> Memory;
        vector<Ptr<noeud>> cluster_members;
};

TypeId noeud::GetTypeId()
{
	static TypeId tid = TypeId("noeud")
	                    .SetParent<Node> ()
	                    .AddConstructor<noeud> ();
	return tid;
}



void Set_Position(NodeContainer noeuds)
{
	// Initialisation d'un objet de positionnement
	Ptr<PositionAllocator> positionAlloc = CreateObject<RandomRectanglePositionAllocator> ();
	positionAlloc->SetAttribute ("X", StringValue ("ns3::UniformRandomVariable[Min=5|Max=95]"));
	positionAlloc->SetAttribute ("Y", StringValue ("ns3::UniformRandomVariable[Min=5|Max=95]"));

	// Création d'un objet MobilityHelper et ajout du modèle de positionnement
	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel");
	mobility.SetPositionAllocator (positionAlloc);
	//mobility.Install (&n);


	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
	                           "Bounds", RectangleValue (Rectangle (0, 100, 0, 100)),
	                           "Distance", DoubleValue(10));

	mobility.Install (noeuds);
}


void SetNewRandomNumbersToNch(vector<Ptr<noeud>> nch)
{
	for(Ptr<noeud> n : nch)
	{
		n->r_randomnumber=(double)rand()/RAND_MAX;//affectation d'un random number entre 0 et 1
	}
}


vector<Ptr<noeud>> SetCH(double tr, vector<Ptr<noeud>> nch, vector<Ptr<noeud>> CH)
{
	CH.clear();
	for(uint i = 0 ; i < nch.size() ; i++)
	{
		if(nch[i]->r_randomnumber < tr)
		{
			CH.push_back(nch[i]);
		}
	}
	return CH;
}

void CH_Characteristic(Ptr<noeud> n ,AnimationInterface* an)
{
	an->UpdateNodeColor(n->GetId() , 0 , 255 , 0);
	an->UpdateNodeDescription(n , "Cluster Head");
	an->UpdateNodeSize(n->GetId() , 5 , 5);

}

void NCH_Characteristic(vector<Ptr<noeud>> nch ,AnimationInterface* an)
{
	for (uint i = 0 ; i < nch.size() ; i++)
	{
		an->UpdateNodeColor(nch[i]->GetId() , 255 , 255 , 0);
		an->UpdateNodeDescription(nch[i] , "Cant be Cluster Head");
		an->UpdateNodeSize(nch[i]->GetId() , 3 , 3);
	}
}

void nch_Characteristic(vector<Ptr<noeud>> nch ,AnimationInterface* an)
{
	for (uint i = 0 ; i < nch.size() ; i++)
	{
		an->UpdateNodeColor(nch[i]->GetId() , 255 , 0 , 0);
		an->UpdateNodeDescription(nch[i] , "Not Cluster Head yet");
		an->UpdateNodeSize(nch[i]->GetId() , 1 , 1);
	}
}

void network_Characteristic(vector<Ptr<noeud>> nch ,AnimationInterface* an)
{
	for (uint i = 0 ; i < nch.size() ; i++)
	{
		an->UpdateNodeColor(nch[i]->GetId() , 255 , 0 , 0);
		an->UpdateNodeDescription(nch[i] , "Not Cluster Head yet");
		an->UpdateNodeSize(nch[i]->GetId() , 1 , 1);
	}
}


//avoir l'indice du noeud n dans le tableau tab
int GetNodeIndex(Ptr<noeud> n,vector<Ptr<noeud>> tab)
{
	int index=0;
	int taille = tab.size();
	while((tab[index]!=n) && (index < taille))
	{
		index+=1;
	}
	return index;
}


void SendPacketToNCH(Ptr<Socket> socket, Vector pos)//position="(x,y)"
{
	std::string message ="("+to_string(pos.x)+","+to_string(pos.y)+")";

	Ptr<Packet> packet=Create<Packet>((uint8_t*) message.c_str(), message.size());
  	socket->Send(packet);
  	socket->Close();
}


void ReceivePacket (Ptr<Socket> socket)
{
	Ptr<Packet> packet;
	uint8_t buffer[1024];
	std::string mes;

  	while (packet = socket->Recv ())
    {
    	packet->CopyData(buffer,1024);
    	cout << "le noeud avec l'id " << socket->GetNode()->GetId() << " a recu " << buffer <<endl;
    	// Enlever les parenthèses
    	/*
    	mes = mes.substr(1, mes.size() - 2);

    	std::istringstream iss(mes);
	    std::string val;

	    // Tableau pour stocker les deux valeurs
	    double arr[2];
	    int i = 0;

	    while (std::getline(iss, val, ',')) 
	    {
            // Convertir chaque valeur en double et l'ajouter au tableau
            arr[i] = std::stod(val);
            ++i;
        }

        std::vector<double> newVec (arr, arr + sizeof(arr) / sizeof(double));
        DynamicCast<noeud>(socket->GetNode())->Memory.push_back(newVec);
        */
    }
}


Ptr<noeud> FindClosestNode(Ptr<noeud> referenceNode, vector<Ptr<noeud>> CH) 
{
    Ptr<MobilityModel> refMob = referenceNode->GetObject<MobilityModel>();
    Vector refPos = refMob->GetPosition();

    double minDistance = std::numeric_limits<double>::max();
    Ptr<noeud> closestNode;

    for (uint i = 0 ; i < CH.size() ; i++) 
    {
        Ptr<MobilityModel> mob = CH[i]->GetObject<MobilityModel>();
        Vector pos = mob->GetPosition();

        double distance = CalculateDistance(refPos, pos);

        if (distance < minDistance) 
        {
            minDistance = distance;
            closestNode = CH[i];
        }
    }

    return closestNode;
}


//calculer la moyenne globale d'énergie du réseau
float total_energy(vector<Ptr<noeud>> network)
{
	float moyenne=0;
	for (uint i = 0 ; i < network.size() ; i++)
	{
		moyenne += static_cast<float>(network[i]->GetObject<LiIonEnergySource>()->GetRemainingEnergy()) / static_cast<float>(INIT_ENERGY);
	}
	moyenne = static_cast<float>(moyenne)/static_cast<float>(network.size());
	return moyenne*100;
}


//return un vector avec tous les noeuds du reseau qui ne sont pas ch le round r
vector<Ptr<noeud>> N_C_H(vector<Ptr<noeud>> CH ,vector<Ptr<noeud>> network , vector<Ptr<noeud>> NCH)
{
	NCH.clear();
	NCH = network;
	for(Ptr<noeud> n : CH)
	{
		NCH.erase(NCH.begin()+GetNodeIndex(n,NCH));       
	}
	return NCH;
}


void install_p2p (vector<Ptr<noeud>> CH , PointToPointHelper p2p)
{

	for (uint i = 0 ; i < CH.size() ; i++)
	{
		vector<Ptr<noeud>> clust = CH[i]->cluster_members;
		for (uint j = 0 ; j < clust.size() ; j++)
		{
			p2p.Install(CH[i] , clust[j]);
		}
	}
}


void dispose_p2p (vector<Ptr<noeud>> CH)
{
	for (uint i = 0 ; i < CH.size() ; i++)
	{
		for (uint j = 0 ; j < CH[i]->GetNDevices() ; j++)
		{
			if (CH[i]->GetDevice(j)->IsPointToPoint())
			{
				DynamicCast<Object>(CH[i]->GetDevice(j))->Dispose();
			}
		}
	}
}



void affichage (float tot_energ , int r)
{
	cout << "l'energie moyenne totale du reseau est de : " << tot_energ << "< " << SEUIL << "%"<< endl;
	cout << "Leach a survécu pendant " << r << " rounds." << endl;
}


void Generate_Traffic(vector<Ptr<noeud>> network , AnimationInterface* anim , PointToPointHelper p2p)
{

	//ouvrir un fichier
	ofstream myfile;
	myfile.open("Clusters.csv" , ios::trunc);

	// Vérification si le fichier est ouvert avec succès
   	if (myfile.is_open()) 
   	{
	   	std::cout << "Le fichier a été ouvert avec succès." << std::endl;
   	} 
   	else 
   	{
       	std::cout << "Impossible d'ouvrir le fichier." << std::endl;
   	}

	vector<Ptr<noeud>> CH;//Cluster Heads
	vector<Ptr<noeud>> nch;//not cluster head yet
	vector<Ptr<noeud>> NCH;//not cluster head in round r
	vector<Ptr<Socket>> NCH_socket_vector;//les socket
	vector<Ptr<Socket>> CH_socket_vector;//les socket

	std::string message = "I am Cluster Head!";
	Ptr<Packet> packet=Create<Packet>((uint8_t*) message.c_str(), message.size());

	//choisir le type de protocole de transport
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	
	//on définit le cp de base
	double cp ;
	//on définit le threshold de base
	double Tr ;
	//Création d'un booléen
	bool tournee = false;
		
	
	nch=network;

	int r = 1;
	
	float tot_energ = total_energy(network);

	while( tot_energ >= SEUIL )
	{
		cout << "on commence une nouvelle tournee." << endl;
		cp = CLUSTER_PERCENTAGE;
		Tr = CLUSTER_PERCENTAGE;

		cout << "cp " << cp <<endl;

		nch=network;

		Simulator::Schedule (Seconds(r) , &network_Characteristic, nch , anim);

		SetNewRandomNumbersToNch(nch);

		CH = SetCH(Tr , nch , CH);

		while ((CH.size() < 0.15*NUM_NODES) || (CH.size() > 0.25*NUM_NODES))
		{
			SetNewRandomNumbersToNch(nch);
			CH = SetCH(Tr , nch , CH);
		}

		//ajouter les id des CH dans le file

		for(Ptr<noeud> n : CH)
		{
			nch.erase(nch.begin()+GetNodeIndex(n,nch)); 
			myfile << n->GetId() << ";";
		}

		myfile <<";\n";


		NCH = N_C_H(CH ,network ,NCH);

		for (uint i = 0 ; i < NCH.size() ; i++)
		{
			NCH[i]->GetObject<LiIonEnergySource>()->DecreaseRemainingEnergy(0.01);
		}

		for (uint i = 0 ; i < CH.size() ; i++)
		{
			CH[i]->GetObject<LiIonEnergySource>()->DecreaseRemainingEnergy(0.02);
		}


		for (uint i = 0 ; i < CH.size() ; i++)
		{
			for (uint j = 0 ; j < NCH.size() ; j++)
			{
				Ptr<Socket> recvSink = Socket::CreateSocket (NCH[j], tid);//initialise un socket pour le noeud e
				InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 20);
				recvSink->Bind (local);
				recvSink->Listen();
				NCH_socket_vector.push_back(recvSink);
				recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
			}

			Ptr<Socket> source = Socket::CreateSocket (CH[i] , tid);
			CH_socket_vector.push_back(source);
			InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 20);
			source->SetAllowBroadcast (true);
			source->Connect (remote);

			Vector pos = CH[i]->GetObject<MobilityModel> ()->GetPosition();
			Simulator::Schedule (Seconds(r+i*0.001) , &SendPacketToNCH,source, pos);
			Simulator::Schedule (Seconds(r+i*0.001) , &CH_Characteristic,CH[i], anim);
			Simulator::Schedule (Seconds(r+i*0.001) , &NCH_Characteristic, NCH , anim);
			Simulator::Schedule (Seconds(r+i*0.001) , &nch_Characteristic, nch , anim);
		}

		//on cherche le CH le plus proche pour chaque noeud NCH
		for (uint i = 0 ; i < NCH.size() ; i++)
		{
			Ptr<noeud> n = FindClosestNode(NCH[i] , CH);
			for (uint j = 0 ; j < CH.size() ; j++)
			{
				if (n->GetId() == CH[j]->GetId())
				{
					CH[j]->cluster_members.push_back(n);
				}
			}
		}

		//on installe des connexions D2D entre cluster head et ses membres de cluster 
		Simulator::Schedule (Seconds(r+CH.size()*0.001) , &install_p2p , CH , p2p);
		//on rompt cette connexion 
		//Simulator::Schedule (Seconds(r+1) , &dispose_p2p , CH);

		//réinitialiser la mémoire 
		for (uint i = 0 ; i < network.size() ; i++)
		{
			network[i]->Memory.clear();
		}

		cp = static_cast<double>(CH.size())/static_cast<double>(NUM_NODES);

		r++;
		tournee=false;
		while(tournee == false)
		{
			if (cp < 0.75)
			{
				SetNewRandomNumbersToNch(nch);
				CH = SetCH(Tr , nch , CH);

				while ((CH.size() < 15) || (CH.size() > 25))
				{
					SetNewRandomNumbersToNch(nch);
					CH = SetCH(Tr , nch , CH);
				}
			}
			else 
			{
				CH = nch;
			}

			//ajouter les id des CH dans le file

			for(Ptr<noeud> n : CH)
			{
				nch.erase(nch.begin()+GetNodeIndex(n,nch));
				myfile << n->GetId() << ";";
			}

			myfile <<";\n";

			NCH = N_C_H(CH ,network ,NCH);

			for (uint i = 0 ; i < NCH.size() ; i++)
			{
				NCH[i]->GetObject<LiIonEnergySource>()->DecreaseRemainingEnergy(0.01);
			}

			for (uint i = 0 ; i < CH.size() ; i++)
			{
				CH[i]->GetObject<LiIonEnergySource>()->DecreaseRemainingEnergy(0.02);
			}

			for (uint i = 0 ; i < CH.size() ; i++)
			{
				for (uint j = 0 ; j < NCH.size() ; j++)
				{
					Ptr<Socket> recvSink = Socket::CreateSocket (NCH[j], tid);//initialise un socket pour le noeud e
					InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 20);
					recvSink->Bind (local);
					recvSink->Listen();
					NCH_socket_vector.push_back(recvSink);
					recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
				}

				Ptr<Socket> source = Socket::CreateSocket (CH[i] , tid);
				CH_socket_vector.push_back(source);
				InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 20);
				source->SetAllowBroadcast (true);
				source->Connect (remote);

				Vector pos = CH[i]->GetObject<MobilityModel> ()->GetPosition();
				Simulator::Schedule (Seconds(r+i*0.001) , &SendPacketToNCH,source, pos);
				Simulator::Schedule (Seconds(r+i*0.001) , &CH_Characteristic,CH[i], anim);
				Simulator::Schedule (Seconds(r+i*0.001) , &NCH_Characteristic,NCH , anim);
				Simulator::Schedule (Seconds(r+i*0.001) , &nch_Characteristic, nch , anim);
			}

			//on installe des connexions D2D entre cluster head et ses membres de cluster 
			Simulator::Schedule (Seconds(r+CH.size()*0.001) , &install_p2p , CH , p2p);
			//on rompt cette connexion 
			//Simulator::Schedule (Seconds(r+1) , &dispose_p2p , CH );

			//réinitialiser la mémoire 
			for (uint i = 0 ; i < network.size() ; i++)
			{
				network[i]->Memory.clear();
			}


			cp += static_cast<double>(CH.size())/static_cast<double>(NUM_NODES);
			cout << "cp " << cp << endl;
			r++;//On incremente le nombre de rounds
			tot_energ = total_energy(network);
			if (nch.empty() == 1)
			{
			  tournee = true;
			}
		}
		tot_energ = total_energy(network);
	}

	Simulator::Schedule (Seconds(99.99) , &affichage , tot_energ , r);

	myfile.close();
}


int main(int argc , char* argv[])
{
	//définir un vector pour stocker les noeuds du réseau
	vector<Ptr<noeud>> network;


	// Création d'un container
	NodeContainer noeuds;
	//création du réseau 
	for (uint32_t i = 0 ; i < NUM_NODES ; i++)
	{
		Ptr<noeud> node = CreateObject<noeud> ();
		network.push_back(node);
		noeuds.Add(node);
	}

	Set_Position(noeuds);

	//créé des connexions 
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

	// Installation du protocole Internet sur les nœuds
	InternetStackHelper stack;
	stack.Install (noeuds);

	//======================================WIFI AD HOC===============================================
	WifiHelper wifi;
	wifi.SetStandard(ns3::WIFI_PHY_STANDARD_80211n_5GHZ);



	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();


	wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(-50));
	//wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	//wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(50));

	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();

	// This is one parameter that matters when using FixedRssLossModel
	// set it to zero; otherwise, gain will be added

	wifiPhy.Set ("RxGain", DoubleValue (0) );

	wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

	wifiPhy.SetChannel (wifiChannel.Create ());

	// Add a mac and disable rate control
	WifiMacHelper wifiMac;
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
	                              "DataMode",StringValue ("DsssRate1Mbps"),
	                              "ControlMode",StringValue ("DsssRate1Mbps"));

	wifiMac.SetType ("ns3::AdhocWifiMac");
	NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, noeuds);

	//==============================================================================================
	
	//=============================================Ipv4=============================================
	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.1.1.0", "255.255.255.0");

	Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);
	//==============================================================================================
	
	//==============================================================================================

	for (uint i = 0 ; i < network.size() ; ++i)
	{
		Ptr<LiIonEnergySource> energySource = CreateObject<LiIonEnergySource>();
		// Configurer les paramètres de la source d'énergie Li-ion 
		energySource->SetInitialEnergy(INIT_ENERGY);
		network[i]->AggregateObject(energySource);
	}

	AnimationInterface anim("LEACH.xml");

	Simulator::ScheduleNow(&Generate_Traffic , network , &anim , p2p);

	Simulator::Stop (Seconds (100.0));
	// Exécution de la simulation
	Simulator::Run ();

	// Arrêt de la simulation
	Simulator::Destroy ();
	return 0;
}
