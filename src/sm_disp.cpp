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

#include <vector>
#include <string>
#include <unordered_map>
#include <regex>
#include <time.h>

#include "sm_disp.hpp"

smfigapl::uint smfigapl::SmDisplay::getSpacing(bool large)
{
	if (large)
		return spacingLarge_;
	else
		return spacing_;
}

void smfigapl::SmDisplay::displayStatisticSimple()
{
	std::cout << std::left << std::setw(spacingLarge_) << "Team:";
	for (uint i = 0; i < input->setup.realCapacity.NTeams; i++)
	{
		std::cout << std::setw(spacing_) << input->setup.namedTeamByTeamId[i].getTeamName();
	}
	std::cout << std::endl;
	for (uint i=0;i<statistic->getNKeys();i++)
	{
		std::cout << std::left << std::setw(this->getSpacing(true)) << statistic->getHeader(statistic->getKey(i));
		for (uint j = 0; j < input->setup.realCapacity.NTeams; j++)
		{
			std::cout << std::setw(this->getSpacing(false)) << statistic->getEntry(statistic->getKey(i),j);
		}
		std::cout << std::endl;
	}
}


void smfigapl::SmDisplay::displayGamePlanSimple()
{
	uint NTournaments=1;
	if (input->conditions.repeatPairings)
		NTournaments=output->indexedGamePlan.getNAtomic();
	for (uint l=0;l<NTournaments;l++)
	{
		if (input->conditions.repeatPairings)
			std::cout<< input->setup.lang.get("tournament")<<": "<<l+1<<std::endl;
		for (uint i = 0; i < output->indexedGamePlan.getNRounds(l); i++)
		{
				std::cout << input->setup.lang.get("round") << ": " << i + 1 << std::endl;
			for (uint j = 0; j < input->setup.realCapacity.NFields; j++)
			{
				if (output->indexedGamePlan.getAtomicIndex(j)==l ||!input->conditions.repeatPairings)
				{
					std::cout << std::right << std::setw(spacingLarge_) << output->namedGamePlan.getTeamFullName(i,j,0) + " : ";
					std::cout << std::left << std::setw(spacingLarge_) << output->namedGamePlan.getTeamFullName(i,j,1);
					std::cout << std::endl;
				}
			}
			if (output->namedGamePlan.getNPauseTeams(i) > 0)
			{
				std::cout << std::right << std::setw(spacingLarge_) << input->setup.lang.get("break") + ": ";
				for (uint k = 0; k < output->namedGamePlan.getNPauseTeams(i); k++)
				{
					std::cout << output->namedGamePlan.getPauseTeam(i,k);
					if (k < output->namedGamePlan.getNPauseTeams(i) - 1)
						std::cout << ", ";
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
	}
}
void smfigapl::SmDisplay::setSize(uint x , uint y)
{
	outTableSizex_=x;
	outTableSizey_=y;
	outTable_.resize(y);
	for (auto &&r:outTable_)
		r.resize(x);
}

void smfigapl::SmDisplay::setOutTable(uint x , uint y, std::string s)
{
	this->setOutTable(x, y, s,false);
}


void smfigapl::SmDisplay::setOutTable(uint x , uint y, std::string s, bool overWrite)
{
	if (outTableSizey_<y+1)
	{
		outTable_.resize(y+1);
		for (uint i=outTableSizey_;i<y+1;i++)
			outTable_[i].resize(outTableSizex_);
		outTableSizey_=y+1;
	}
	if (outTableSizex_<x+1)
	{
		for (uint i=0;i<outTableSizey_;i++)
			outTable_[i].resize(x+1);
		outTableSizex_=x+1;
	}
	if (overWrite)
		outTable_[y][x]=s;
	else
	{
		std::string old=outTable_[y][x];
		if (!old.empty())
			old+="\t";
		outTable_[y][x]=old+s;
	}

}

void smfigapl::SmDisplay:: displayGamePlanFull()
{
	setSize(134,18);

	SmPos pos=input->format.infoFields.basePos;

	this->setOutTable(pos.x+input->format.infoFields.resultMessage.xRel, pos.y+input->format.infoFields.resultMessage.yRel, output->getResultMessage());
	if (output->isSolved())
	{
		// Info fields
		for (auto& [k, v]: input->format.infoFields.general)
		{
			this->setOutTable(pos.x+v.xRel, pos.y+v.yRel, input->info.general.values[k]);
			if (input->format.settings.showFilterCollumn)
				this->setOutTable(pos.x+input->format.settings.filterCollumn, pos.y+v.yRel, "1", true);
		}
		//date_time
		uint startTime;
		uint roundTime;
		uint breakTime;
		for (auto& [k, v]: input->format.infoFields.dateTime)
		{
			std::string ft=input->format.infoFields.dateTime[k].formatString;
			SmDateTimeValue dt=input->info.dateTime.values[k];
			struct tm tim;
			tim.tm_year=dt.year-1900;
			tim.tm_mon=dt.month-1;
			tim.tm_mday=dt.day;
			tim.tm_hour=dt.hour;
			tim.tm_min=dt.minute;
			tim.tm_sec=dt.second;
			if (k=="start_time")
				startTime=dt.hour*3600+dt.minute*60+dt.second;
			if (k=="round_time")
				roundTime=dt.hour*3600+dt.minute*60+dt.second;
			if (k=="break_time")
				breakTime=dt.hour*3600+dt.minute*60+dt.second;
			char dts [50];
			strftime (dts,50,ft.c_str(),&tim);
			std::string dt_string = static_cast<std::string>(dts);
			this->setOutTable(pos.x+v.xRel, pos.y+v.yRel, dt_string);
			if (input->format.settings.showFilterCollumn)
				this->setOutTable(pos.x+input->format.settings.filterCollumn, pos.y+v.yRel, "1", true);

			if (input->format.infoFields.dateTimeHeader.find(k)!=input->format.infoFields.dateTimeHeader.end())
			{
				this->setOutTable(pos.x+v.xRel+input->format.infoFields.dateTimeHeader[k].xRel, pos.y+v.yRel+input->format.infoFields.dateTimeHeader[k].yRel, input->format.infoFields.dateTimeHeader[k].text);
				if (input->format.settings.showFilterCollumn)
					this->setOutTable(pos.x+input->format.settings.filterCollumn, pos.y+v.yRel+input->format.infoFields.dateTimeHeader[k].yRel, "1", true);

			}
		}
		std::vector<std::string> startTimeArray(input->setup.realCapacity.NRounds);
		std::vector<std::string> endTimeArray(input->setup.realCapacity.NRounds);
		for (uint i=0;i<input->setup.realCapacity.NRounds;i++)
		{
			std::string ft=input->format.infoFields.dateTime["round_time"].formatString;
			uint absTim=startTime+i*roundTime+(i>0?i*breakTime:0);
			struct tm tim;
			tim.tm_hour=absTim/3600;
			tim.tm_min=(absTim-tim.tm_hour*3600)/60;
			tim.tm_sec=(absTim-tim.tm_hour*3600-tim.tm_min*60);
			char dts [50];
			strftime (dts,50,ft.c_str(),&tim);
			std::string dt_string = static_cast<std::string>(dts);
			startTimeArray[i]=dt_string;
			absTim +=roundTime;
			tim.tm_hour=absTim/3600;
			tim.tm_min=(absTim-tim.tm_hour*3600)/60;
			tim.tm_sec=(absTim-tim.tm_hour*3600-tim.tm_min*60);
			strftime (dts,50,ft.c_str(),&tim);
			dt_string = static_cast<std::string>(dts);
			endTimeArray[i]=dt_string;

		}

		//teams

		if(input->format.settings.showTeams)
		{
			uint xFac=1;
			uint yFac=1;
			if (input->format.settings.teamsUseCollumn)
				xFac=0;
			else
				yFac=0;

			this->setOutTable(pos.x+input->format.teams.basePos.x+input->format.teams.title.xRel, pos.y+input->format.teams.basePos.y+input->format.teams.title.yRel,input->format.teams.title.text);
			for (uint i=0;i<input->setup.realCapacity.NTeams;i++)
			{
				this->setOutTable(pos.x+input->format.teams.basePos.x+i*xFac, pos.y+input->format.teams.basePos.y+i*yFac,input->setup.namedTeamByTeamId[i].getTeamName());
				if (input->format.settings.showFilterCollumn)
					this->setOutTable(pos.x+input->format.settings.filterCollumn, pos.y+input->format.teams.basePos.y+i*yFac, "1", true);
			}
			if(input->format.settings.showTeamsTextColors)
			{
				this->setOutTable(pos.x+input->format.teams.basePos.x+input->format.teams.textColors.title.xRel, pos.y+input->format.teams.basePos.y+input->format.teams.textColors.title.yRel,input->format.teams.textColors.title.text);
				if (input->format.settings.showFilterCollumn)
					this->setOutTable(pos.x+input->format.settings.filterCollumn, pos.y+input->format.teams.basePos.y, "1", true);
				for (uint i=0;i<input->setup.realCapacity.NTeams;i++)
				{
					this->setOutTable(pos.x+input->format.teams.basePos.x+input->format.teams.textColors.posRel.x+i*xFac, pos.y+input->format.teams.basePos.y+input->format.teams.textColors.posRel.y+i*yFac,input->format.teams.textColors.colors[i]);
					if (input->format.settings.showFilterCollumn)
						this->setOutTable(pos.x+input->format.settings.filterCollumn, pos.y+input->format.teams.basePos.y+input->format.teams.textColors.posRel.y+i*yFac, "1", true);
				}
			}
		}

		//GamePlan
		uint NTournaments=input->format.settings.fixedTournaments;
		if (input->format.settings.separateTournaments)
			NTournaments=std::max(NTournaments,output->indexedGamePlan.getNAtomic());
		uint roundFactor=std::max(input->format.settings.fixedFields,input->setup.realCapacity.NFields);
		uint tournamentFactor=std::max(input->format.settings.fixedRounds,input->setup.realCapacity.NRounds)*roundFactor+input->format.tournament.relPos.yRel+input->format.tournament.collumns.relPos.yRel;
		for (uint l=0;l<NTournaments;l++)
		{
			bool active=false;
			uint tournamentOffset=l*tournamentFactor;
			uint colBase=pos.x+input->format.gamePlan.basePos.x+input->format.tournament.relPos.xRel+input->format.tournament.collumns.relPos.xRel;
			uint rowBase=pos.y+input->format.gamePlan.basePos.y+input->format.tournament.relPos.yRel+input->format.tournament.collumns.relPos.yRel+tournamentOffset;
			for (uint i = 0; i < output->indexedGamePlan.getNRounds(l); i++)
			{
				bool once =false;
				bool firstRow =false;
				for (uint j = 0; j < input->setup.realCapacity.NFields; j++)
				{
					if (output->indexedGamePlan.getAtomicIndex(j)==l)
					{

						uint row=rowBase+i*roundFactor+j;
						if (input->format.settings.showFilterCollumn)
							this->setOutTable(pos.x+input->format.settings.filterCollumn, row, "1", true);
						if (!output->indexedGamePlan.getPause(i, j))
						{
							active=true;
							this->setOutTable(colBase+input->format.tournament.collumns.col["home_team"].xRel,
									row,output->namedGamePlan.getTeamFullName(i,j,0));
							if (input->format.settings.showTeamsTextColors)
							{
								if (input->format.settings.useTeamColor)
								{
									std::string homeColor=output->namedGamePlan.getTeamColor(i, j, 0);
									std::transform(homeColor.begin(), homeColor.end(), homeColor.begin(), ::tolower);
									this->setOutTable(colBase+input->format.tournament.collumns.col["home_team_text_color"].xRel,
										row,homeColor);
									std::string awayColor=output->namedGamePlan.getTeamColor(i, j, 1);
									std::transform(awayColor.begin(), awayColor.end(), awayColor.begin(), ::tolower);
									this->setOutTable(colBase+input->format.tournament.collumns.col["away_team_text_color"].xRel,
										row,awayColor);
								}
								else
								{
									this->setOutTable(colBase+input->format.tournament.collumns.col["home_team_text_color"].xRel,
										row,input->format.teams.textColors.colors[output->indexedGamePlan.getGame(i, j).getTeambyGameId(0).getTeamId()]);
									this->setOutTable(colBase+input->format.tournament.collumns.col["away_team_text_color"].xRel,
										row,input->format.teams.textColors.colors[output->indexedGamePlan.getGame(i, j).getTeambyGameId(1).getTeamId()]);
								}
							}
							this->setOutTable(colBase+input->format.tournament.collumns.col["away_team"].xRel,
									row,output->namedGamePlan.getTeamFullName(i,j,1));
						}
							this->setOutTable(colBase+input->format.tournament.collumns.col["field_name"].xRel,
									row,input->setup.fieldNameByFieldId[j]);
							this->setOutTable(colBase+input->format.tournament.collumns.col["field_type"].xRel,
									row,input->setup.fieldTypeNameByFieldId[j]);
						if (!once&&active)
						{
							once=true;
							uint rowTime=rowBase+i*roundFactor;
							if (!firstRow)
								rowTime+=j;
							this->setOutTable(colBase+input->format.tournament.collumns.col["round"].xRel,
														rowTime,std::to_string(i+1));
							this->setOutTable(colBase+input->format.tournament.collumns.col["start_time"].xRel,
													rowTime,startTimeArray[i]);
							this->setOutTable(colBase+input->format.tournament.collumns.col["end_time"].xRel,
													rowTime,endTimeArray[i]);
						}
					}
				}
				if (output->namedGamePlan.getNPauseTeams(i) > 0)
				{
					for (uint k = 0; k < output->namedGamePlan.getNPauseTeamsAtomic(i,l); k++)
					{
						this->setOutTable(colBase+input->format.tournament.collumns.col["pause_teams"].xRel,
												rowBase+i*roundFactor+k,output->namedGamePlan.getPauseTeamAtomic(i,k,l));
						if (input->format.settings.showFilterCollumn)
							this->setOutTable(pos.x+input->format.settings.filterCollumn, rowBase+i*roundFactor+k, "1", true);
						if (input->format.settings.showTeamsTextColors)
						{
							if (input->format.settings.useTeamColor)
							{
								std::string pauseColor=output->namedGamePlan.getPauseTeamColorAtomic(i,k,l);
								std::transform(pauseColor.begin(), pauseColor.end(), pauseColor.begin(), ::tolower);
								this->setOutTable(colBase+input->format.tournament.collumns.col["pause_teams_text_color"].xRel,
										rowBase+i*roundFactor+k,pauseColor);

							}
							else
							{
//								this->setOutTable(colBase+input->format.tournament.collumns.col["pause_team_text_color"].xRel,
//										rowBase+i*roundFactor+k,input->format.teams.textColors.colors[output->indexedGamePlan.getPauseAtomic(i, k, l)].ge);
							}
						}
					}
				}
			}
			//title
			if (active)
			{
				std::string ts =std::regex_replace(input->format.tournament.title.text, std::regex("\\$0"),std::to_string(l+1));
				this->setOutTable(pos.x+input->format.gamePlan.basePos.x+input->format.tournament.relPos.xRel+input->format.tournament.title.xRel,
						pos.y+input->format.gamePlan.basePos.y+input->format.tournament.relPos.yRel+tournamentOffset+input->format.tournament.title.yRel,ts);
				if (input->format.settings.showFilterCollumn)
					this->setOutTable(pos.x+input->format.settings.filterCollumn, pos.y+input->format.gamePlan.basePos.y+input->format.tournament.relPos.yRel+tournamentOffset+input->format.tournament.title.yRel, "1", true);
				for (auto& [k, v]: input->format.tournament.collumns.col)
				{
					this->setOutTable(colBase+v.xRel,
							rowBase+v.yRel,v.text);
					if (input->format.settings.showFilterCollumn)
						this->setOutTable(pos.x+input->format.settings.filterCollumn, rowBase+v.yRel, "1", true);
				}
			}
		}

		//Statistic
		if(input->format.settings.showStatistic)
		{
			uint xFac=1;
			uint yFac=1;
			if (input->format.settings.statisticUseCollumn)
				xFac=0;
			else
				yFac=0;
			uint colBase=pos.x+input->format.statistic.basePos.x;
			uint rowBase=pos.y+input->format.statistic.basePos.y;
			this->setOutTable(colBase+input->format.statistic.title.xRel,
					rowBase+input->format.statistic.title.yRel,input->format.statistic.title.text);
			if (input->format.settings.showFilterCollumn)
				this->setOutTable(pos.x+input->format.settings.filterCollumn, rowBase+input->format.statistic.title.yRel, "1", true);
			for (uint i = 0; i < input->setup.realCapacity.NTeams; i++)
				{
				this->setOutTable(colBase+input->format.statistic.rows.relPos.xRel+i*xFac-yFac,
					rowBase+input->format.statistic.rows.relPos.yRel-xFac+i*yFac,input->setup.namedTeamByTeamId[i].getTeamName());
				if (input->format.settings.showFilterCollumn)
					this->setOutTable(pos.x+input->format.settings.filterCollumn, rowBase+input->format.statistic.rows.relPos.yRel-1*xFac+yFac, "1", true);
				}
			uint fieldTypeOffset=0;
			bool updateFieldTypeOffset;
			for (uint i = 0; i < statistic->getNKeys(); i++)
			{
				updateFieldTypeOffset=false;
				auto k=statistic->getKey(i);
				if (k.substr(0,11)=="field_types")
				{
					k="field_types";
					updateFieldTypeOffset=true;
				}
				std::string rowHeader=std::regex_replace(input->format.statistic.rows.row[k].text, std::regex("\\$0"),statistic->getHeader(statistic->getKey(i)));
				this->setOutTable(colBase+input->format.statistic.rows.relPos.xRel+input->format.statistic.rows.row[k].xRel+fieldTypeOffset*yFac,
					rowBase+input->format.statistic.rows.relPos.yRel+input->format.statistic.rows.row[k].yRel+fieldTypeOffset*xFac,rowHeader);
				if (input->format.settings.showFilterCollumn)
					this->setOutTable(pos.x+input->format.settings.filterCollumn,rowBase+input->format.statistic.rows.relPos.yRel+input->format.statistic.rows.row[k].yRel, "1", true);
				for (uint j = 0; j < input->setup.realCapacity.NTeams; j++)
				{
					this->setOutTable(colBase+input->format.statistic.rows.relPos.xRel+i*yFac+j*xFac,
						rowBase+input->format.statistic.rows.relPos.yRel+i*xFac+j*yFac,std::to_string(statistic->getEntry(statistic->getKey(i),j)));
					if (input->format.settings.showFilterCollumn)
						this->setOutTable(pos.x+input->format.settings.filterCollumn,rowBase+input->format.statistic.rows.relPos.yRel+i*xFac+j*yFac, "1", true);
				}
				if (updateFieldTypeOffset)
					fieldTypeOffset++;
			}
		}

		//fixedRows
		if (input->format.settings.showFilterCollumn)
			for (auto && r:input->format.settings.fixedRows)
			{
				this->setOutTable(pos.x+input->format.settings.filterCollumn,r, "1", true);
			}
	}



//	for (auto&& row:outTable_)
//	{
//		for (auto i=row.begin();i<row.end();i++)
//		{
//			if (*i=="")
//				*i="";
//		}
//	}

	//Display
	for (auto&& row:outTable_)
	{
		for (auto i=row.begin();i<row.end();i++)
		{
			std::cout<<*i;
			if (i<row.end())
				std::cout<<separator_;
		}
		std::cout <<std::endl;
	}
}


