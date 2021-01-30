// https://github.com/kaiberggold/smfigapl
//
// MIT License
//
// Copyright 2020 Kai Berggold
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>
#include "sm_file.hpp"

void smfigapl::GamePlanFile::displayErrorMessage(const toml::parse_error& err)
{
	#ifdef SM_DBG
		std::cerr<<input->setup.lang.get("errorId")<<": "<<TOML_PARSE_ERROR<<". "<<input->setup.lang.getErrorMessage(TOML_PARSE_ERROR)<<" Toml Error: "<<err<<std::endl;
	#else
		std::cerr<<input->setup.lang.get("errorId")<<": "<<TOML_PARSE_ERROR<<". "<<input->setup.lang.getErrorMessage(TOML_PARSE_ERROR)<<std::endl;
	#endif
}

void smfigapl::GamePlanFile::readFile(std::string fileName, std::string cfgFileName)
{
	try
	{
		inTable_ = toml::parse_file(fileName);
		inTableCfg_ = toml::parse_file(cfgFileName);
 		return;
	}
	catch (const toml::parse_error& err)
	{
		this->displayErrorMessage(err);
	}
	catch (const SmValueException& e)
	{
		 std::cout << e.what();
		 exit (-1);
	}
}

void smfigapl::GamePlanFile::readFile(std::string fileName)
{
	try
	{
		inTable_ = toml::parse_file(fileName);
		inTableCfg_ = inTable_;
 		return;
	}
	catch (const toml::parse_error& err)
	{
		this->displayErrorMessage(err);
	}
	catch (const SmValueException& e)
	{
		 std::cout << e.what();
		 exit (-1);
	}
}

void smfigapl::GamePlanFile::readStdIn()
{
	try
	{
		std::string s;
		for (std::string line; std::getline(std::cin, line);)
		{
			s+=line;
			s+="\n";
		}
		inTable_=toml::parse(s);
 		return;
	}
	catch (const toml::parse_error& err)
	{
		this->displayErrorMessage(err);
		exit(-1);
	}
	catch (const SmValueException& e)
	{
		 std::cout << e.what();
		 exit (-1);
	}
}


void smfigapl::GamePlanFile::getBool(toml::v2::table *t, std::string key, bool *value, bool required, uint errorId)
{
	bool isSet=false;
	std::vector<std::string> trueStrings=input->setup.lang.getAllWithCapitalization("true");
	std::vector<std::string> falseStrings=input->setup.lang.getAllWithCapitalization("false");
	for (auto && s:input->setup.lang.getAllWithCapitalization(key))
			{
				if ((*t)[s].is_boolean())
				{
					*value=static_cast<bool>((*t)[s].as_boolean()->get());
					isSet=true;
					break;
				}
				std::string v;
				if ((*t)[s].is_string())
					v=(*t)[s].as_string()->get();
				for (auto && s:trueStrings)
					if (s==v)
					{
						*value=true;
						isSet=true;
						break;
					}
				for (auto && s:falseStrings)
					if (s==v)
					{
						*value=false;
						isSet=true;
						break;
					}
			}
	if (!isSet && required)
		#ifdef SM_DBG
			throw SmValueException("",errorId,{""},input);
		#else
			throw SmValueException(errorId,{""},input);
		#endif
}

void smfigapl::GamePlanFile::getBool(toml::v2::table *t, std::string key, bool *value)
{
	this->getBool(t, key, value, false, 0);
}

template<class Type> void  smfigapl::GamePlanFile::get(toml::v2::table *t, std::string key, Type *value, bool required, uint errorId)
{
	bool isSet=false;
	#ifdef SM_DBG
	if (input->dbg&DBG_FILE)
		std::cout << *t << "\n";
	#endif
	for (auto && s:input->setup.lang.getAllWithCapitalization(key))
	{
		if constexpr (std::is_same_v<Type, std::string>)
		{
			if ((*t)[s].is_string())
			{
				*value=static_cast<std::string>((*t)[s].as_string()->get());
				isSet=true;
				break;
			}
		}
		else if ((*t)[s].is_number())
			{
				if constexpr (std::is_same_v<Type, uint>)
						*value=static_cast<uint>((*t)[s].as_integer()->get());
				if constexpr (std::is_same_v<Type, int>)
						*value=static_cast<int>((*t)[s].as_integer()->get());
				isSet=true;
				break;
			}
	}
		if (!isSet && required)
			#ifdef SM_DBG
				throw SmValueException("",errorId,{""},input);
			#else
				throw SmValueException(NROUNDS_NOT_SET,{""},input);
			#endif
}

template<class Type> void smfigapl::GamePlanFile::get(toml::v2::table *t, std::string key, Type *value)
{
	this->get(t, key, value, false, 0);
}

toml::v2::table *smfigapl::GamePlanFile::getTable(std::vector<std::string>  keys)
{
	toml::v2::table *t=&inTable_;
	for (auto &&k:keys)
	{
		bool exists=false;
		for (auto && s:input->setup.lang.getAllWithCapitalization(k))
			if ((*t)[s].is_table())
			{
				exists=true;
				t=(*t)[s].as_table();
				break;
			}
		if (!exists)
			return nullptr;
	}
	return t;
}

toml::v2::array *smfigapl::GamePlanFile::getArray(toml::v2::table *table,std::string key)
{
	for (auto && s:input->setup.lang.getAllWithCapitalization(key))
		if ((*table)[s].is_array())
		{
			return(*table)[s].as_array();
		}
	return nullptr;
}

void smfigapl::GamePlanFile::parseFile()
{
	try
	{
		bool isSet;
		//NRounds, required
		uint n;
		this->get(&inTable_,"rounds", &n, true,NROUNDS_NOT_SET);
		input->setNRounds(n);

		//Filescheme, optional
		uint file_scheme;
		this->get(&inTable_,"file_scheme", &file_scheme, false,0);

		//iterations, optional
		this->get(&inTable_,"iterations", &input->conditions.iterations, false,0);

		//repeatedPairings, optional
		this->getBool(&inTable_,"repeat_pairings", &input->conditions.repeatPairings);

		//home_away_balance
		this->getBool(&inTable_,"home_away_balance", &input->conditions.homeAwayBalance);

		//Fields
		//TODO use new table function
		std::string fn;
		isSet=false;
		for (auto && l:input->setup.lang.getLanguages())
		{
			for (auto && s:input->setup.lang.getAllWithCapitalization("fields",l))
			{
				if (inTable_[s].is_array())
				{
					isSet=true;
					fn=input->setup.lang.get("fields",l);
					std::string tn=input->setup.lang.get("type",l);
					std::string stn=input->setup.lang.get("subtype",l);
					std::string nn=input->setup.lang.get("name",l);

					toml::array& fields = *inTable_.get_as<toml::array>(fn);
					for (auto && field : fields)
					{
						auto ft=field.as_table()->get(tn)->as_string()->get();
						auto st=field.as_table()->get(stn)->as_string()->get();
						auto name=field.as_table()->get(nn)->as_string()->get();
						input->addField(ft,st,name);
					}
				}
			}
		}
		if (!isSet)
			#ifdef SM_DBG
				throw SmValueException("",NO_FIELDS_DEFINED,{fn},input);
			#else
				throw SmValueException(NO_FIELDS_DEFINED,p,input);
			#endif

		//Clubs
		isSet=false;
		std::string cln;
		for (auto && l:input->setup.lang.getLanguages())
		{
			for (auto && s:input->setup.lang.getAllWithCapitalization("clubs",l))
			{
				if (inTable_[s].is_array())
				{
					isSet=true;
					cln=input->setup.lang.get("clubs",l);
					std::string cn=input->setup.lang.get("colors",l);
					std::string nn=input->setup.lang.get("name",l);
					toml::array& clubs = *inTable_.get_as<toml::array>(cln);
					for (auto && club : clubs)
					{
						auto clubName=club.as_table()->get(nn)->as_string()->get();
						auto colors=*club.as_table()->get_as<toml::array>(cn);
						for (auto && color : colors)
						{
							auto clubColor=color.as_string()->get();
							input->addTeam(static_cast<std::string>(clubName),static_cast<std::string>(clubColor));
						}
					}
				}
			}
		}
		if (!isSet)
			#ifdef SM_DBG
				throw SmValueException("",NO_CLUBS_DEFINED,{cln},input);
			#else
				throw SmValueException(NO_CLUBS_DEFINED,p,input);
			#endif

		//Conditions
		for (auto && l:input->setup.lang.getLanguages())
		{
			for (auto && s:input->setup.lang.getAllWithCapitalization("conditions",l))
			{
				if (inTable_[s].is_table())
				{
					std::string pgn=input->setup.lang.get("pause_games",l);
					std::string scn=input->setup.lang.get("same_club",l);
					std::string efn=input->setup.lang.get("empty_fields",l);
					std::string mn=input->setup.lang.get("min",l);
					std::string man=input->setup.lang.get("max",l);
					std::string ftn=input->setup.lang.get("field_types",l);
					std::string nn=input->setup.lang.get("name",l);
					std::string stsn=input->setup.lang.get("subtypes",l);
					std::string tdn=input->setup.lang.get("teams_deny",l);

					if (inTable_[s][pgn].is_table())
					{
						input->conditions.setMaxPauseGames(static_cast<uint>(inTable_[s][pgn][man].as_integer()->get()));
					}
					if (inTable_[s][scn].is_table())
					{
						input->conditions.setmaxGamesSameClub(static_cast<uint>(inTable_[s][scn][man].as_integer()->get()));
					}
					if (inTable_[s][efn].is_table())
					{
						input->conditions.setMaxFieldsEmpty(static_cast<uint>(inTable_[s][efn][man].as_integer()->get()));
					}
					if (inTable_[s][ftn].is_array())
					{
						toml::array& ftcs = *inTable_[s][ftn].as_array();
						input->conditions.limGamesFieldType.enabled=true;
						for (auto && ftc : ftcs)
								{
									uint min,max;
									if (ftc.as_table()->get(mn)->is_number())
										min=static_cast<uint>(ftc.as_table()->get(mn)->as_integer()->get());
									if (ftc.as_table()->get(man)->is_number())
										 max=static_cast<uint>(ftc.as_table()->get(man)->as_integer()->get());
									if (max<min)
									{
										std::vector<std::string> p={std::to_string(min),std::to_string(max)};
										#ifdef SM_DBG
											throw SmValueException("min: "+std::to_string(min)+" max:"+std::to_string(max),INPUT_ERROR_MAX_LT_MIN,p,input);
										#else
											throw SmValueException(INPUT_ERROR_MAX_LT_MIN,p,input);
										#endif
									}
									auto name=ftc.as_table()->get(nn)->as_string()->get();
									input->setMinGamesFieldType(static_cast<std::string>(name),min);
									input->setMaxGamesFieldType(static_cast<std::string>(name),max);
								}
					}

					if (inTable_[s][stsn].is_array())
					{
						toml::array& sts = *inTable_[s][stsn].as_array();

						for (auto && st : sts)
						{
							uint fst=input->setup.fieldSubTypeIdBySubTypeName[st.as_table()->get(nn)->as_string()->get()];
							toml::array&  tds=*st.as_table()->get(tdn)->as_array();
							uint size=static_cast<uint>(tds.size());
							if (size%2==0)
								{
								input->conditions.fieldSubTypeDeny.enabled=true;
								for (uint i=0;i<size>>1;i++)
								{
									std::string s=tds[i<<1].as_string()->get()+tds[(i<<1)+1].as_string()->get();
									uint id=input->setup.teamIdArray[input->setup.clubIdByClubName[tds[i<<1].as_string()->get()]][input->setup.colorIdByColor[tds[(i<<1)+1].as_string()->get()]];
									input->conditions.fieldSubTypeDeny.values[id][fst]=true;

								}

							}
						}
					}
				}
				//TODO: else
			}
		}

		inTable_=inTableCfg_;
		//information
		if (auto t=this->getTable({"information","general"}))
		{
			input->info.general.values.clear();
			for (auto i = t->begin(); i != t->end(); i++)
			{
				input->info.general.values[i->first]=i->second.as_string()->get();
			}

		}
		if (auto t=this->getTable({"information","date_time"}))
		{
			for (auto i = t->begin(); i != t->end(); i++)
			{
				if (i->second.is_date_time())
				{
					auto dateTime=i->second.as_date_time()->get();
					input->info.dateTime.values[i->first].year=dateTime.date.year;
					input->info.dateTime.values[i->first].month=dateTime.date.month;
					input->info.dateTime.values[i->first].day=dateTime.date.day;
					input->info.dateTime.values[i->first].hour=dateTime.time.hour;
					input->info.dateTime.values[i->first].minute=dateTime.time.minute;
					input->info.dateTime.values[i->first].second=dateTime.time.second;
				}
				else if (i->second.is_date())
				{
					auto date=i->second.as_date()->get();
					input->info.dateTime.values[i->first].year=date.year;
					input->info.dateTime.values[i->first].month=date.month;
					input->info.dateTime.values[i->first].day=date.day;
				}
				else if (i->second.is_time())
				{
					auto time=i->second.as_time()->get();
					input->info.dateTime.values[i->first].hour=time.hour;
					input->info.dateTime.values[i->first].minute=time.minute;
					input->info.dateTime.values[i->first].second=time.second;
				}
			}
		}

		//format.settings
		if (auto t=this->getTable({"format","settings"}))
		{
			this->getBool(t, "separate_tournaments", &input->format.settings.separateTournaments);
			this->getBool(t, "show_teams", &input->format.settings.showTeams);
			this->getBool(t, "show_teams_text_colors", &input->format.settings.showTeamsTextColors);
			this->getBool(t, "teams_use_collumn", &input->format.settings.teamsUseCollumn);
			this->getBool(t, "statistic_use_collumn", &input->format.settings.statisticUseCollumn);
			this->getBool(t, "show_statistic", &input->format.settings.showStatistic);
			this->getBool(t, "show_filter_collumn", &input->format.settings.showFilterCollumn);
			this->getBool(t, "use_team_color", &input->format.settings.useTeamColor);

			this->get(t, "fixed_fields", &input->format.settings.fixedFields);
			this->get(t, "fixed_rounds", &input->format.settings.fixedRounds);
			this->get(t, "fixed_tournaments", &input->format.settings.fixedTournaments);
			this->get(t, "filter_collumn", &input->format.settings.filterCollumn);
			if (auto arr=this->getArray(t,"filter_collumn_fixed_rows"))
				for (auto && a : *arr)
				{
					input->format.settings.fixedRows.push_back(static_cast<uint>(a.as_integer()->get()));
				}
		}

		//format.information
		if (auto t=this->getTable({"format","information","base_pos"}))
		{
			this->get(t, "pos_x", &input->format.infoFields.basePos.x);
			this->get(t, "pos_y", &input->format.infoFields.basePos.y);
		}
		//format.information.result_message
		if (auto t=this->getTable({"format","information","result_message"}))
		{
			this->get(t, "pos_rel_x", &input->format.infoFields.resultMessage.xRel);
			this->get(t, "pos_rel_y", &input->format.infoFields.resultMessage.yRel);
		}

		//format.information.text_fields
		if (auto t=this->getTable({"format","information"}))
		{
			if (auto arr=this->getArray(t,"text_fields"))
			{
				for (auto && a : *arr)
				{
					std::string type;
					this->get(a.as_table(),"type",&type,true,0);
					this->get(a.as_table(), "pos_rel_x", &input->format.infoFields.general[type].xRel,true,0);
					this->get(a.as_table(), "pos_rel_y", &input->format.infoFields.general[type].yRel,true,0);
				}
			}

			//format.information.date_time
			if (auto arr=this->getArray(t,"date_time"))
			{
				for (auto && a : *arr)
				{
					std::string type;
					this->get(a.as_table(),"type",&type,true,0);
					this->get(a.as_table(),"format_string",&input->format.infoFields.dateTime[type].formatString);
					this->get(a.as_table(), "pos_rel_x", &input->format.infoFields.dateTime[type].xRel,true,0);
					this->get(a.as_table(), "pos_rel_y", &input->format.infoFields.dateTime[type].yRel,true,0);			}
			}

			//format.information.date_time_header
			if (auto arr=this->getArray(t,"date_time_header"))
			{
				for (auto && a : *arr)
				{
					std::string type;
					this->get(a.as_table(),"type",&type,true,0);
					this->get(a.as_table(),"text",&input->format.infoFields.dateTimeHeader[type].text);
					this->get(a.as_table(), "pos_rel_x", &input->format.infoFields.dateTimeHeader[type].xRel,true,0);
					this->get(a.as_table(), "pos_rel_y", &input->format.infoFields.dateTimeHeader[type].yRel,true,0);
				}
			}
		}

		//format.teams
		if (auto t=this->getTable({"format","teams"}))
		{
			if (auto t=this->getTable({"format","teams","base_pos"}))
			{
				this->get(t, "pos_x", &input->format.teams.basePos.x);
				this->get(t, "pos_y", &input->format.teams.basePos.y);
			}
			if (auto t=this->getTable({"format","teams","title"}))
			{
				this->get(t, "pos_rel_x", &input->format.teams.title.xRel);
				this->get(t, "pos_rel_y", &input->format.teams.title.yRel);
				this->get(t, "text", &input->format.teams.title.text);
			}
			if (auto t=this->getTable({"format","teams","text_color"}))
			{
				if (auto arr=this->getArray(t,"colors"))
				{
					for (auto && a : *arr)
					{
						input->format.teams.textColors.colors.push_back(a.as_string()->get());
					}
					//fill up or clip
					//TODO: except if no color defined
					uint size=static_cast<uint>(input->format.teams.textColors.colors.size());
					if (size>input->setup.realCapacity.NTeams)
						input->format.teams.textColors.colors.resize(input->setup.realCapacity.NTeams);
					else if(size<input->setup.realCapacity.NTeams)
					{
						input->format.teams.textColors.colors.resize(input->setup.realCapacity.NTeams);
						for (uint i=size;i<input->setup.realCapacity.NTeams;i++)
							input->format.teams.textColors.colors[i]=input->format.teams.textColors.colors[i%size];
					}
				}
				this->get(t, "pos_rel_x", &input->format.teams.textColors.posRel.x);
				this->get(t, "pos_rel_y", &input->format.teams.textColors.posRel.y);
			}
			if (auto t=this->getTable({"format","teams","text_color", "title"}))
			{
				this->get(t, "pos_rel_x", &input->format.teams.textColors.title.xRel);
				this->get(t, "pos_rel_y", &input->format.teams.textColors.title.yRel);
				this->get(t, "text", &input->format.teams.textColors.title.text);
			}
		}

		//format.gameplan
		if (auto t=this->getTable({"format","game_plan"}))
		{
			if (auto t=this->getTable({"format","game_plan","base_pos"}))
			{
				this->get(t, "pos_x", &input->format.gamePlan.basePos.x);
				this->get(t, "pos_y", &input->format.gamePlan.basePos.y);
			}
			if (auto t=this->getTable({"format","game_plan","title"}))
				{
					this->get(t, "pos_rel_x", &input->format.gamePlan.title.xRel);
					this->get(t, "pos_rel_y", &input->format.gamePlan.title.yRel);
					this->get(t, "text", &input->format.gamePlan.title.text);
				}
		}

		//format.tournament
		if (auto t=this->getTable({"format","tournament"}))
		{
			if (auto t=this->getTable({"format","tournament","rel_pos"}))
			{
				this->get(t, "pos_rel_x", &input->format.tournament.relPos.xRel);
				this->get(t, "pos_rel_y", &input->format.tournament.relPos.yRel);
			}
			if (auto t=this->getTable({"format","tournament","title"}))
			{
				this->get(t, "pos_rel_x", &input->format.tournament.title.xRel);
				this->get(t, "pos_rel_y", &input->format.tournament.title.yRel);
				this->get(t, "text", &input->format.tournament.title.text);
			}
			if (auto t=this->getTable({"format","tournament","collumns"}))
			{
				if (auto t=this->getTable({"format","tournament","collumns","rel_pos"}))
				{
					this->get(t, "pos_rel_x", &input->format.tournament.collumns.relPos.xRel);
					this->get(t, "pos_rel_y", &input->format.tournament.collumns.relPos.yRel);
				}
				if (auto arr=this->getArray(t,"collumn"))
				{
					for (auto && a : *arr)
					{
						std::string type;
						this->get(a.as_table(),"type",&type,true,0);
						this->get(a.as_table(),"text",&input->format.tournament.collumns.col[type].text);
						this->get(a.as_table(), "pos_rel_x", &input->format.tournament.collumns.col[type].xRel,true,0);
						this->get(a.as_table(), "pos_rel_y", &input->format.tournament.collumns.col[type].yRel,true,0);
					}
				}
			}
		}

		//format.statistic
		if (auto t=this->getTable({"format","statistic"}))
		{
			if (auto t=this->getTable({"format","statistic","base_pos"}))
			{
				this->get(t, "pos_x", &input->format.statistic.basePos.x);
				this->get(t, "pos_y", &input->format.statistic.basePos.y);
			}
			if (auto t=this->getTable({"format","statistic","title"}))
			{
				this->get(t, "pos_rel_x", &input->format.statistic.title.xRel);
				this->get(t, "pos_rel_y", &input->format.statistic.title.yRel);
				this->get(t, "text", &input->format.statistic.title.text);
			}
			if (auto t=this->getTable({"format","statistic","rows"}))
			{
				if (auto t=this->getTable({"format","statistic","rows","rel_pos"}))
				{
					this->get(t, "pos_rel_x", &input->format.statistic.rows.relPos.xRel);
					this->get(t, "pos_rel_y", &input->format.statistic.rows.relPos.yRel);
				}
				if (auto arr=this->getArray(t,"row"))
				{
					for (auto && a : *arr)
					{
						std::string type;
						this->get(a.as_table(),"type",&type,true,0);
						this->get(a.as_table(),"text",&input->format.statistic.rows.row[type].text);
						this->get(a.as_table(), "pos_rel_x", &input->format.statistic.rows.row[type].xRel,true,0);
						this->get(a.as_table(), "pos_rel_y", &input->format.statistic.rows.row[type].yRel,true,0);
					}
				}
			}
		}

		return;
	}
	catch (const toml::parse_error& err)
	{
		std::cerr << "Parsing failed:\n" << err << "\n";
		exit(-1);
	}
	catch (const SmValueException& e)
	{
		 std::cout << e.what();
		 exit (-1);
	}
}
