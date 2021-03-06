/*

	Copyright 2018 Michal Fratczak

	This file is part of habdec.

    habdec is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    habdec is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with habdec.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <string>
#include <iostream>
#include <fstream>
#include <regex>

#include <boost/program_options.hpp>

#include "GLOBALS.h"
#include "common/console_colors.h"
#include "habitat/habitat_interface.h"

namespace
{

int LoadPayloadParameters(std::string i_payload_id)
{
	using namespace std;
	using namespace habdec::habitat;

	std::map<std::string, HabitatFlight> flights = ListFlights(0);

	for(auto& flight : flights)
	{
		for(auto& payload : flight.second.payloads_)
		{
			if( !i_payload_id.compare(payload.second.id_) )
			{
				GLOBALS::get().baud_ = payload.second.baud_;
				GLOBALS::get().rtty_ascii_bits_ = payload.second.ascii_bits_;
				GLOBALS::get().rtty_ascii_stops_ = payload.second.ascii_stops_;
				GLOBALS::get().frequency_ = payload.second.frequency_;

				cout<<C_MAGENTA<<"Loading parameters for payload "<<i_payload_id<<C_OFF<<endl;

				cout<<"\tbaud: "<<payload.second.baud_<<endl;
				cout<<"\tascii_bits: "<<payload.second.ascii_bits_<<endl;
				cout<<"\tascii_stops: "<<payload.second.ascii_stops_<<endl;
				cout<<"\tfrequency: "<<payload.second.frequency_<<endl;

				return 1;
			}
		}
	}
	return 0;
}

} // namespace

void prog_opts(int ac, char* av[])
{
	namespace po = boost::program_options;
	using namespace std;

	try
	{
		po::options_description generic("CLI opts");
		generic.add_options()
			("help", "Display help message")
			("device",	po::value<int>()->default_value(-1), "SDR Device Numer")
			("sampling_rate",	po::value<double>()->default_value(0), "Sampling Rate, as supported by device")

			("port",	po::value<string>()->default_value("5555"),	"Command Port, example: --port 127.0.0.1:5555")

			("station",	po::value<string>()->default_value(""),	"HABHUB station callsign")
			("latlon",	po::value< std::vector<float> >()->multitoken(), "station GPS location (decimal)")

			("freq",	po::value<float>(), "frequency in MHz")
			("ppm",		po::value<float>(), "frequency correction in PPM")
			("gain",	po::value<int>(), "gain")
			("print",   po::value<bool>(), "live print received chars, values: 0, 1")
			("rtty",	po::value< std::vector<float> >()->multitoken(), "rtty: baud bits stops, example: -rtty 300 8 2")
			("biast",	po::value<bool>(), "biasT, values: 0, 1")
			("bias_t",	po::value<bool>(), "biasT, values: 0, 1")
			("afc",		po::value<bool>(), "Auto Frequency Correction, values: 0, 1")

			("sentence_cmd",	po::value<string>(), "Call external command with sentence as parameter")

			("flights",	po::value<int>()->implicit_value(0), "List Habitat flights")
			("payload",	po::value<string>(), "Configure for Payload ID")
		;

		po::options_description cli_options("Command Line Interface options");
		cli_options.add(generic);

		string config_file;
		cli_options.add_options()
            ("config", po::value<string>(&config_file), "Last run config file. Autosaved on every successfull decode.");
            // ("config", po::value<string>(&config_file)->default_value("./habdecWebsocketServer.opts"), "Last run config file. Autosaved on every successfull decode.");

		po::options_description file_options;
        file_options.add(generic);

		po::variables_map vm;
		store( po::command_line_parser(ac, av).options(cli_options).allow_unregistered().run(), vm );
		notify(vm);

		if(vm.count("help"))
        {
            cout<<cli_options<<endl;
			exit(0);
		}

		if(config_file != "")
		{
			ifstream ifs(config_file.c_str());
			if (!ifs)
			{
				cout << "Can not open config file: " << config_file << endl;
			}
			else
			{
				cout<<C_RED<<"Reading config from file "<<config_file<<C_OFF<<endl;
				store(parse_config_file(ifs, file_options, 1), vm);
				notify(vm);
			}
		}

		if (vm.count("device"))
		{
			GLOBALS::get().device_ = vm["device"].as<int>();
		}
		if (vm.count("sampling_rate"))
		{
			GLOBALS::get().sampling_rate_ = vm["sampling_rate"].as<double>();
		}
		if (vm.count("port")) // [host:][port]
		{
			smatch match;
			regex_match( vm["port"].as<string>(), match, std::regex(R"_(([\w\.]*)(\:?)(\d*))_") );

			if(match.size() == 4)
			{
				if(match[2] == "" && match[3] == "") // special case when only port is given: --port 5555
				{
					GLOBALS::get().command_port_ = stoi(match[1]);
				}
				else
				{
					if(match[1] != "")	GLOBALS::get().command_host_ = match[1];
					if(match[3] != "")	GLOBALS::get().command_port_ = stoi(match[3]);
				}
			}
		}
		if (vm.count("station"))
		{
			GLOBALS::get().station_callsign_ = vm["station"].as<string>();
		}
		if (vm.count("payload"))
		{
			GLOBALS::get().habitat_payload_ = vm["payload"].as<string>();
			if( !LoadPayloadParameters( GLOBALS::get().habitat_payload_ ) )
				cout<<C_RED<<"Failed loading payload "<<GLOBALS::get().habitat_payload_ <<endl;
		}
		if (vm.count("sentence_cmd"))
		{
			GLOBALS::get().sentence_cmd_ = vm["sentence_cmd"].as<string>();
		}
		if (vm.count("freq"))
		{
			GLOBALS::get().frequency_ = vm["freq"].as<float>() * 1e6;
		}
		if (vm.count("ppm"))
		{
			GLOBALS::get().ppm_ = vm["ppm"].as<float>();
		}
		if (vm.count("gain"))
		{
			GLOBALS::get().gain_ = vm["gain"].as<int>();
		}
		if (vm.count("print"))
		{
			GLOBALS::get().live_print_ = vm["print"].as<bool>();
		}
		if (vm.count("biast"))
		{
			GLOBALS::get().biast_ = vm["biast"].as<bool>();
		}
		if (vm.count("bias_t"))
		{
			GLOBALS::get().biast_ = vm["bias_t"].as<bool>();
		}
		if (vm.count("afc"))
		{
			GLOBALS::get().afc_ = vm["afc"].as<bool>();
		}
		if (vm.count("rtty"))
		{
			vector<float> rtty_tokens = vm["rtty"].as< vector<float> >();
			if( rtty_tokens.size() != 3 )
			{
				cout<<C_RED<<"--rtty option needs 3 args: baud ascii-bits stop-bits"<<C_OFF<<endl;
				exit(1);
			}

			if(rtty_tokens[2] != 1 && rtty_tokens[2] != 2)
			{
				cout<<C_RED<<"Only 1 or 2 stop bits are supported."<<C_OFF<<endl;
				exit(1);
			}
			if(rtty_tokens[1] != 7 && rtty_tokens[1] != 8)
			{
				cout<<C_RED<<"ASCII Bits must be 7 or 8"<<C_OFF<<endl;
				exit(1);
			}

			GLOBALS::get().baud_ = rtty_tokens[0];
			GLOBALS::get().rtty_ascii_bits_ = rtty_tokens[1];
			GLOBALS::get().rtty_ascii_stops_ = rtty_tokens[2];
		}
		if (vm.count("flights"))
		{
			using namespace habdec::habitat;
			int hours_offset = vm["flights"].as<int>();
			cout<<"Habitat Flights: "<<endl;
			std::map<std::string, HabitatFlight> payloads = ListFlights(hours_offset);
			for(auto& flight : payloads)
				cout<<flight.second<<endl;
			exit(0);
		}
		if (vm.count("latlon"))
		{
			vector<float> latlon_vec = vm["latlon"].as< vector<float> >();
			if( latlon_vec.size() != 2 )
			{
				cout<<C_RED<<"--latlon option needs 2 args"<<C_OFF<<endl;
				exit(1);
			}
			GLOBALS::get().station_lat_ = latlon_vec[0];
			GLOBALS::get().station_lon_ = latlon_vec[1];
		}
	}
	catch(exception& e)
	{
		cout << e.what() << "\n";
	}

	GLOBALS::DumpToFile("./habdecWebsocketServer.opts");
}