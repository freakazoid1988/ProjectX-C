//
//  AI.cpp
//  ProjectX
//
//  Created by Gaetano Belcastro on 03/02/15.
//  Copyright (c) 2015 GRG. All rights reserved.
//

#include <string>
#include "Scacchiera.cpp"
#include <unordered_map>
#include <cassert>
#include <cmath>
#include <iostream>
#include <ctype.h>
#include <limits>
#include <cstring>
#include <boost/algorithm/string.hpp>

using namespace::std;
static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required");


class AI{
    
private:
    
    string mossaFinale;
    const short CostiCattura[14] = { 0, 8, 12, 16, 24, 36, 100, 100, 100, 100, 100, 100, 100, 100 };
    const int minColumn[10] = { 1, 1, 1, 1, 1, 1, 2, 3, 4, 5}; //da che colonna inizia la scacchiera per ogni riga compresa cornice
    const int maxColumn[10] = { 5, 5, 6, 7, 8, 9, 9, 9, 9, 9}; //a che colonna finisce la scacchiera per ogni riga compresa cornice
    short direzioni[6] = {1,2,3,4,5,6};//N,NO,O,S,SE,E
    unordered_map<int, int> distance;
    //int distance[11][11][11][11];
    int dist;
    const short white=2, black=3;
    Scacchiera scacchiera;
    
    int calcolaDistanza(int i, int j, int k, int l){
        int col = j-l;
        int riga = i-k;
        if((col<0) ^ (riga <0))
            return (abs(col)+abs(riga));
        return max(abs(col), abs(riga));
    }
    
  
    
    char corrispondenzaR(int indice) {
        switch(indice){
            case 1:
                return 'A';
            case 2:
                return 'B';
            case 3:
                return 'C';
            case 4:
                return 'D';
            case 5:
                return 'E';
            case 6:
                return 'F';
            case 7:
                return 'G';
            case 8:
                return 'H';
            case 9:
                return 'I';
            default:
                throw;
        }
    }
    
    int corrispondenza(char indice) {
        //char x = char(toupper(indice));
        switch(toupper(indice)){
            case 'A':
                return 1;
            case 'B':
                return 2;
            case 'C':
                return 3;
            case 'D':
                return 4;
            case 'E':
                return 5;
            case 'F':
                return 6;
            case 'G':
                return 7;
            case 'H':
                return 8;
            case 'I':
                return 9;
            default:
                throw;
        }
    }
    
    double calcolaCoesione(short *copia, int i, int j) {
        double val = 0;
        short side = copia[i*11+j];
        if(copia[(i-1)*11+j] == side)//Nord
            val+= 0.1;
        if(copia[(i-1)*11+(j-1)] == side)//Nord-Ovest
            val+= 0.1;
        if(copia[i*11+(j-1)] == side)//Ovest
            val+= 0.1;
        if(copia[i*11+(j+1)] == side)//Est
            val+= 0.1;
        if(copia[(i+1)*11+j] == side)//Sud
            val+= 0.1;
        if(copia[(i+1)*11+(j+1)] == side)//Sud-Est
            val+= 0.1;
        
        return val;
    }
    
    double valutaMossa(Scacchiera scacchiera2, string side1, int depth, double alfabeta) {
    		//numMosse++;
    		short* scacc= scacchiera2.getScacchiera();
    		short s1, s2;
    		string side2, m, mossa;
    		if(side1.at(1)=='h'){
    			s1 = white;
    			s2 = black;
    			side2 = "black";
    		}
    		else {
    			s1 = black;
    			s2 = white;
    			side2 = "white";
    		}

    		if(depth == 0){
    			//assegna valore a configurazione corrente
    			double w1 = 1,w2 = 1,w3 = 1,w4 = 50; //pesi
    			double centerDist = 0, coesione = 0, premioCatt = 0, penaleCatt = 0;
    			if(s1==2){
    				//FIXME // mangia 7 pedine bianche e da errore
    				premioCatt = CostiCattura[scacchiera2.getNereCatturate()];
    				penaleCatt = CostiCattura[scacchiera2.getBiancheCatturate()];
    			}else{
    				premioCatt = CostiCattura[scacchiera2.getBiancheCatturate()];
    				penaleCatt = CostiCattura[scacchiera2.getNereCatturate()];
    			}
    			for(int i = 1; i<10; i++)
    				for(int j = minColumn[i]; j <= maxColumn[i]; j++){
    					if(scacc[i*11+j] == s1){
                            std::unordered_map<int,int>::const_iterator got = distance.find(i*1000+j*100+5*10+5);
    						centerDist += 2/((got->second) + 1);
    						coesione += calcolaCoesione(scacc,i,j);
    					}else if(scacc[i*11+j] == s2){
                            std::unordered_map<int,int>::const_iterator got = distance.find(i*1000+j*100+5*10+5);
    						centerDist -= 5/((got->second) + 1);
    						coesione -= 1.5*calcolaCoesione(scacc,i,j);
    					}
    				}
    			return (w1*centerDist) + (w2*coesione) + (w3*premioCatt) - (w4*penaleCatt);
    		}else{
    			//genera configurazione futura
    			double bestValue = std::numeric_limits<double>::infinity(), currValue, ab = alfabeta;
    			//string m, mossa;
    			Scacchiera scacFuturaClass;
    			//short* scac = scacchiera2.getScacchiera();
    			short* scacFutura;
    			for(int i=1; i<10; i++){
    					for(int j = minColumn[i]; j<=maxColumn[i]; j++){
    						if(scacc[i*11+j] == s1){
    							for(int k = 0; k < 6; k++){
    								if(direzioni[k]==1){//NORD
    									if(scacchiera2.esisteCella(i-1, j)){//FIXME
    										if(scacc[(i-1)*11+j] == 1){//una pedina
    											scacFuturaClass = scacchiera2;
    											//scacFutura = scacFuturaClass.getScacchiera();
    											scacFuturaClass.aggiornaScacchiera(i, j, i, j, i-1, j, i-1, j);
    											m = generaStringaMossa(i, j, i, j, i-1, j, i-1, j);
    											currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    											if(currValue < bestValue){
    												bestValue = currValue;
    												mossa = m;
    											}
    											if (alfabeta > bestValue)
    												goto hell;
    											ab = bestValue;
    										}else if(scacc[(i-1)*11+j] == s1){// due pedine allineate
    												if(scacchiera2.esisteCella(i-2, j)){
    													if(scacc[(i-2)*11+j] == 1){// la cella controllata e' vuota quindi mi sposto li
    														scacFuturaClass = scacchiera2;
    														//scacFutura = scacFuturaClass.getScacchiera();
    														scacFuturaClass.aggiornaScacchiera(i-1, j, i, j, i-2, j, i-1, j);//indici invertiti per avere indici mossa ordinati correttaete per tutto il caso NORD
    														m = generaStringaMossa(i-1, j, i, j, i-2, j, i-1, j);
    														currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    														if(currValue < bestValue){
    															bestValue = currValue;
    															mossa = m;
    														}
    														if (alfabeta > bestValue)
    															goto hell;
    														ab = bestValue;
    													}else if(scacc[(i-2)*11+j] == s2 && scacchiera2.esisteCella(i-3, j) && ((scacc[(i-3)*11+j] == 1) || scacc[(i-3)*11+j] == 0 )){//NOTA: if innestati per controllare una sola volta esistenza celle per i-3, i-4 ecc ecc
    															scacFuturaClass = scacchiera2;
    															//scacFutura = scacFuturaClass.getScacchiera();
    															scacFuturaClass.aggiornaScacchiera(i-1, j, i, j, i-2, j, i-1, j);
    															m = generaStringaMossa(i-1, j, i, j, i-2, j, i-1, j);
    															currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    															if(currValue < bestValue){
    																bestValue = currValue;
    																mossa = m;
    															}
    															if (alfabeta > bestValue)
    																goto hell;
    															ab = bestValue;
    														}else if(scacc[(i-2)*11+j] == s1 ){//tre pedine allineate
    															if(scacchiera2.esisteCella(i-3, j)){
    																if(scacc[(i-3)*11+j] == 1){//cella vuota
    																	scacFuturaClass = scacchiera2;
    																	//scacFutura = scacFuturaClass.getScacchiera();
    																	scacFuturaClass.aggiornaScacchiera(i-2, j, i, j, i-3, j, i-1, j);
    																	m = generaStringaMossa(i-2, j, i, j, i-3, j, i-1, j);
    																	currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																	if(currValue < bestValue){
    																		bestValue = currValue;
    																		mossa = m;
    																	}
    																	if (alfabeta > bestValue)
    																		goto hell;
    																	ab = bestValue;
    																}else if(scacc[(i-3)*11+j] == s2 && scacchiera2.esisteCella(i-4, j)){//c'e' avversario
    																		if(scacc[(i-4)*11+j] == 1){
    																			scacFuturaClass = scacchiera2;
    																			//scacFutura = scacFuturaClass.getScacchiera();
    																			scacFuturaClass.aggiornaScacchiera(i-2, j, i, j, i-3, j, i-1, j);
    																			m = generaStringaMossa(i-2, j, i, j, i-3, j, i-1, j);
    																			currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																			if(currValue < bestValue){
    																				bestValue = currValue;
    																				mossa = m;
    																			}
    																			if (alfabeta > bestValue)
    																				goto hell;
    																			ab = bestValue;
    																		}else if(scacc[(i-4)*11+j] == s2){
    																				if(scacchiera2.esisteCella(i-5, j) && scacc[(i-5)*11+j] == 1){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i-2, j, i, j, i-3, j, i-1, j);
    																					m = generaStringaMossa(i-2, j, i, j, i-3, j, i-1, j);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}else if(scacc[(i-5)*11+j] == 0){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i-2, j, i, j, i-3, j, i-1, j);
    																					m = generaStringaMossa(i-2, j, i, j, i-3, j, i-1, j);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}
    																		}
    																	}else if(scacc[(i-3)*11+j] == s2 && scacc[(i-4)*11+j] == 0){
    																		scacFuturaClass = scacchiera2;
    																		//scacFutura = scacFuturaClass.getScacchiera();
    																		scacFuturaClass.aggiornaScacchiera(i-2, j, i, j, i-3, j, i-1, j);
    																		m = generaStringaMossa(i-2, j, i, j, i-3, j, i-1, j);
    																		currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																		if(currValue < bestValue){
    																			bestValue = currValue;
    																			mossa = m;
    																		}
    																		if (alfabeta > bestValue)
    																			goto hell;
    																		ab = bestValue;
    																	}
    																}
    														}
    												}
    										}
    									}
    								}else if(direzioni[k]==2){
    									//NORD-OVEST
    									if(scacchiera2.esisteCella(i-1, j-1)){//FIXME
    										if(scacc[(i-1)*11+j-1] == 1){//una pedina
    											scacFuturaClass = scacchiera2;
    											//scacFutura = scacFuturaClass.getScacchiera();
    											scacFuturaClass.aggiornaScacchiera(i, j, i, j, i-1, j-1, i-1, j-1);
    											m = generaStringaMossa(i, j, i, j, i-1, j-1, i-1, j-1);
    											currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    											if(currValue < bestValue){
    												bestValue = currValue;
    												mossa = m;
    											}
    											if (alfabeta > bestValue)
    												goto hell;
    											ab = bestValue;
    										}else if(scacc[(i-1)*11+j-1] == s1){// due pedine allineate
    												if(scacchiera2.esisteCella(i-2, j-2)){
    													if(scacc[(i-2)*11+j-2] == 1){// la cella controllata e' vuota
    														scacFuturaClass = scacchiera2;
    														//scacFutura = scacFuturaClass.getScacchiera();
    														scacFuturaClass.aggiornaScacchiera(i-1, j-1, i, j, i-2, j-2, i-1, j-1); //indici NO inertiti per lo stesso motivo del NORD
    														m = generaStringaMossa(i-1, j-1, i, j, i-2, j-2, i-1, j-1);
    														currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    														if(currValue < bestValue){
    															bestValue = currValue;
    															mossa = m;
    														}
    														if (alfabeta > bestValue)
    															goto hell;
    														ab = bestValue;
    													}else if(scacc[(i-2)*11+j-2] == s2 && (scacchiera2.esisteCella(i-3, j-3) && (scacc[(i-3)*11+j-3] == 1 || scacc[(i-3)*11+j-3] == 0))){//NOTA: if annestati per controllare una sola volta esistenza celle per i-3, i-4 ecc ecc
    															scacFuturaClass = scacchiera2;
    															//scacFutura = scacFuturaClass.getScacchiera();
    															scacFuturaClass.aggiornaScacchiera(i-1, j-1, i, j, i-2, j-2, i-1, j-1);
    															m = generaStringaMossa(i-1, j-1, i, j, i-2, j-2, i-1, j-1);
    															currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    															if(currValue < bestValue){
    																bestValue = currValue;
    																mossa = m;
    															}
    															if (alfabeta > bestValue)
    																goto hell;
    															ab = bestValue;
    															}else if(scacc[(i-2)*11+j-2] == s1 ){//tre pedine allineate
    															if(scacchiera2.esisteCella(i-3, j-3)){
    																if(scacc[(i-3)*11+j-3] == 1){//cella vuota
    																	scacFuturaClass = scacchiera2;
    																	//scacFutura = scacFuturaClass.getScacchiera();
    																	scacFuturaClass.aggiornaScacchiera(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																	m = generaStringaMossa(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																	currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																	if(currValue < bestValue){
    																		bestValue = currValue;
    																		mossa = m;
    																	}
    																	if (alfabeta > bestValue)
    																		goto hell;
    																	ab = bestValue;
    																}else if(scacc[(i-3)*11+j-3] == s2 && scacchiera2.esisteCella(i-4, j-4)){//c'e' avversario
    																		if(scacc[(i-4)*11+j-4] == 1){
    																			scacFuturaClass = scacchiera2;
    																			//scacFutura = scacFuturaClass.getScacchiera();
    																			scacFuturaClass.aggiornaScacchiera(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																			m = generaStringaMossa(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																			currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																			if(currValue < bestValue){
    																				bestValue = currValue;
    																				mossa = m;
    																			}
    																			if (alfabeta > bestValue)
    																				goto hell;
    																			ab = bestValue;
    																		}else if(scacc[(i-4)*11+j-4] == s2){
    																				if(scacchiera2.esisteCella(i-5, j-5) && scacc[(i-5)*11+j-5] == 1){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																					m = generaStringaMossa(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}else if(scacc[(i-5)*11+j-5] == 0){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																					m = generaStringaMossa(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}
    																		}
    																	}else if(scacc[(i-3)*11+j-3] == s2 && scacc[(i-4)*11+j-4] == 0){
    																		scacFuturaClass = scacchiera2;
    																		//scacFutura = scacFuturaClass.getScacchiera();
    																		scacFuturaClass.aggiornaScacchiera(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																		m = generaStringaMossa(i-2, j-2, i, j, i-3, j-3, i-1, j-1);
    																		currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																		if(currValue < bestValue){
    																			bestValue = currValue;
    																			mossa = m;
    																		}
    																		if (alfabeta > bestValue)
    																			goto hell;
    																		ab = bestValue;
    																	}
    																}
    														}
    												}
    										}
    									}

    								}else if(direzioni[k]==3){
    									//OVEST
    									if(scacchiera2.esisteCella(i, j-1)){//FIXME
    										if(scacc[i*11+j-1] == 1){//una pedina
    											scacFuturaClass = scacchiera2;
    											//scacFutura = scacFuturaClass.getScacchiera();
    											scacFuturaClass.aggiornaScacchiera(i, j, i, j, i, j-1, i, j-1);
    											m = generaStringaMossa(i, j, i, j, i, j-1, i, j-1);
    											currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    											if(currValue < bestValue){
    												bestValue = currValue;
    												mossa = m;
    											}
    											if (alfabeta > bestValue)
    												goto hell;
    											ab = bestValue;
    										}else if(scacc[i*11+j-1] == s1){// due pedine allineate
    												if(scacchiera2.esisteCella(i, j-2)){
    													if(scacc[i*11+j-2] == 1){// la cella controllata e' vuota
    														scacFuturaClass = scacchiera2;
    														//scacFutura = scacFuturaClass.getScacchiera();
    														scacFuturaClass.aggiornaScacchiera(i, j-1, i, j, i, j-2, i, j-1);
    														m = generaStringaMossa(i, j-1, i, j, i, j-2, i, j-1);
    														currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    														if(currValue < bestValue){
    															bestValue = currValue;
    															mossa = m;
    														}
    														if (alfabeta > bestValue)
    															goto hell;
    														ab = bestValue;
    													}else if(scacc[i*11+j-2] == s2 && (scacchiera2.esisteCella(i, j-3) && (scacc[i*11+j-3] == 1 || scacc[i*11+j-3] == 0))){
    															scacFuturaClass = scacchiera2;
    															//scacFutura = scacFuturaClass.getScacchiera();
    															scacFuturaClass.aggiornaScacchiera(i, j-1, i, j, i, j-2, i, j-1);
    															m = generaStringaMossa(i, j-1, i, j, i, j-2, i, j-1);
    															currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    															if(currValue < bestValue){
    																bestValue = currValue;
    																mossa = m;
    															}
    															if (alfabeta > bestValue)
    																goto hell;
    															ab = bestValue;
    															}else if(scacc[i*11+j-2] == s1 ){//tre pedine allineate
    															if(scacchiera2.esisteCella(i, j-3)){
    																if(scacc[i*11+j-3] == 1){//cella vuota
    																	scacFuturaClass = scacchiera2;
    																	//scacFutura = scacFuturaClass.getScacchiera();
    																	scacFuturaClass.aggiornaScacchiera(i, j-2, i, j, i, j-3, i, j-1);
    																	m = generaStringaMossa(i, j-2, i, j, i, j-3, i, j-1);
    																	currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																	if(currValue < bestValue){
    																		bestValue = currValue;
    																		mossa = m;
    																	}
    																	if (alfabeta > bestValue)
    																		goto hell;
    																	ab = bestValue;
    																}else if(scacc[i*11+j-3] == s2 && scacchiera2.esisteCella(i, j-4)){//c'e' avversario
    																		if(scacc[i*11+j-4] == 1){
    																			scacFuturaClass = scacchiera2;
    																			//scacFutura = scacFuturaClass.getScacchiera();
    																			scacFuturaClass.aggiornaScacchiera(i, j-2, i, j, i, j-3, i, j-1);// da controllare
    																			m = generaStringaMossa(i, j-2, i, j, i, j-3, i, j-1);
    																			currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																			if(currValue < bestValue){
    																				bestValue = currValue;
    																				mossa = m;
    																			}
    																			if (alfabeta > bestValue)
    																				goto hell;
    																			ab = bestValue;
    																		}else if(scacc[i*11+j-4] == s2){
    																				if(scacchiera2.esisteCella(i, j-5) && scacc[i*11+j-5] == 1){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i, j-2, i, j, i, j-3, i, j-1);// da controllare, la pedina avversaria non viene toccata
    																					m = generaStringaMossa(i, j-2, i, j, i, j-3, i, j-1);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}else if(scacc[i*11+j-5] == 0){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i, j-2, i, j, i, j-3, i, j-1);// da controllare, la pedina avversaria non viene toccata
    																					m = generaStringaMossa(i, j-2, i, j, i, j-3, i, j-1);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}
    																		}
    																	}else if(scacc[i*11+j-3] == s2 && scacc[i*11+j-4] == 0){
    																		scacFuturaClass = scacchiera2;
    																		//scacFutura = scacFuturaClass.getScacchiera();
    																		scacFuturaClass.aggiornaScacchiera(i, j-2, i, j, i, j-3, i, j-1);// da controllare
    																		m = generaStringaMossa(i, j-2, i, j, i, j-3, i, j-1);
    																		currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																		if(currValue < bestValue){
    																			bestValue = currValue;
    																			mossa = m;
    																		}
    																		if (alfabeta > bestValue)
    																			goto hell;
    																		ab = bestValue;
    																	}
    																}
    														}
    												}
    										}
    									}

    								}else if(direzioni[k]==4){
    									//SUD
    									if(scacchiera2.esisteCella(i+1, j)){//FIXME
    										if(scacc[(i+1)*11+j] == 1){//una pedina
    											scacFuturaClass = scacchiera2;
    											//scacFutura = scacFuturaClass.getScacchiera();
    											scacFuturaClass.aggiornaScacchiera(i, j, i, j, i+1, j, i+1, j);
    											m = generaStringaMossa(i, j, i, j, i+1, j, i+1, j);
    											currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    											if(currValue < bestValue){
    												bestValue = currValue;
    												mossa = m;
    											}
    											if (alfabeta > bestValue)
    												goto hell;
    											ab = bestValue;
    										}else if(scacc[(i+1)*11+j] == s1){// due pedine allineate
    												if(scacchiera2.esisteCella(i+2, j)){
    													if(scacc[(i+2)*11+j] == 1){// la cella controllata e' vuota quindi mi sposto li
    														scacFuturaClass = scacchiera2;
    														//scacFutura = scacFuturaClass.getScacchiera();
    														scacFuturaClass.aggiornaScacchiera(i, j, i+1, j, i+1, j, i+2, j);// da controllare
    														m = generaStringaMossa(i, j, i+1, j, i+1, j, i+2, j);
    														currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    														if(currValue < bestValue){
    															bestValue = currValue;
    															mossa = m;
    														}
    														if (alfabeta > bestValue)
    															goto hell;
    														ab = bestValue;
    													}else if(scacc[(i+2)*11+j] == s2 && (scacchiera2.esisteCella(i+3, j) && (scacc[(i+3)*11+j] == 1 || scacc[(i+3)*11+j] == 0))){//NOTA: if annestati per controllare una sola volta esistenza celle per i-3, i-4 ecc ecc
    															scacFuturaClass = scacchiera2;
    															//scacFutura = scacFuturaClass.getScacchiera();
    															scacFuturaClass.aggiornaScacchiera(i, j, i+1, j, i+1, j, i+2, j);// da controllare, la pedina avversaria non viene toccata
    															m = generaStringaMossa(i, j, i+1, j, i+1, j, i+2, j);
    															currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    															if(currValue < bestValue){
    																bestValue = currValue;
    																mossa = m;
    															}
    															if (alfabeta > bestValue)
    																goto hell;
    															ab = bestValue;
    															}else if(scacc[(i+2)*11+j] == s1 ){//tre pedine allineate
    															if(scacchiera2.esisteCella(i+3, j)){
    																if(scacc[(i+3)*11+j] == 1){//cella vuota
    																	scacFuturaClass = scacchiera2;
    																	//scacFutura = scacFuturaClass.getScacchiera();
    																	scacFuturaClass.aggiornaScacchiera(i, j, i+2, j, i+1, j, i+3, j);// da controllare
    																	m = generaStringaMossa(i, j, i+2, j, i+1, j, i+3, j);
    																	currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																	if(currValue < bestValue){
    																		bestValue = currValue;
    																		mossa = m;
    																	}
    																	if (alfabeta > bestValue)
    																		goto hell;
    																	ab = bestValue;
    																}else if(scacc[(i+3)*11+j] == s2 && scacchiera2.esisteCella(i+4, j)){//c'e' avversario
    																		if(scacc[(i+4)*11+j] == 1){
    																			scacFuturaClass = scacchiera2;
    																			//scacFutura = scacFuturaClass.getScacchiera();
    																			scacFuturaClass.aggiornaScacchiera(i, j, i+2, j, i+1, j, i+3, j);// da controllare
    																			m = generaStringaMossa(i, j, i+2, j, i+1, j, i+3, j);
    																			currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																			if(currValue < bestValue){
    																				bestValue = currValue;
    																				mossa = m;
    																			}
    																			if (alfabeta > bestValue)
    																				goto hell;
    																			ab = bestValue;
    																		}else if(scacc[(i+4)*11+j] == s2){
    																				if(scacchiera2.esisteCella(i+5, j) && scacc[(i+5)*11+j] == 1){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i, j, i+2, j, i+1, j, i+3, j);// da controllare, la pedina avversaria non viene toccata
    																					m = generaStringaMossa(i, j, i+2, j, i+1, j, i+3, j);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}else if(scacc[(i+5)*11+j] == 0){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i, j, i+2, j, i+1, j, i+3, j);// da controllare, la pedina avversaria non viene toccata
    																					m = generaStringaMossa(i, j, i+2, j, i+1, j, i+3, j);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}
    																		}
    																	}else if(scacc[(i+3)*11+j] == s2 && scacc[(i+4)*11+j] == 0){
    																		scacFuturaClass = scacchiera2;
    																		//scacFutura = scacFuturaClass.getScacchiera();
    																		scacFuturaClass.aggiornaScacchiera(i, j, i+2, j, i+1, j, i+3, j);// da controllare
    																		m = generaStringaMossa(i, j, i+2, j, i+1, j, i+3, j);
    																		currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																		if(currValue < bestValue){
    																			bestValue = currValue;
    																			mossa = m;
    																		}
    																		if (alfabeta > bestValue)
    																			goto hell;
    																		ab = bestValue;
    																	}
    																}
    														}
    												}
    										}
    									}
    								}else if(direzioni[k]==5){
    									//SUD-EST
    									if(scacchiera2.esisteCella(i+1, j+1)){//FIXME
    										if(scacc[(i+1)*11+j+1] == 1){//una pedina
    											scacFuturaClass = scacchiera2;
    											//scacFutura = scacFuturaClass.getScacchiera();
    											scacFuturaClass.aggiornaScacchiera(i, j, i, j, i+1, j+1, i+1, j+1);
    											m = generaStringaMossa(i, j, i, j, i+1, j+1, i+1, j+1);
    											currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    											if(currValue < bestValue){
    												bestValue = currValue;
    												mossa = m;
    											}
    											if (alfabeta > bestValue)
    												goto hell;
    											ab = bestValue;
    										}else if(scacc[(i+1)*11+j+1] == s1){// due pedine allineate
    												if(scacchiera2.esisteCella(i+2, j+2)){
    													scacFuturaClass = scacchiera2;
    													//scacFutura = scacFuturaClass.getScacchiera();
    													if(scacc[(i+2)*11+j+2] == 1){// la cella controllata e' vuota quindi mi sposto li'
    														scacFuturaClass.aggiornaScacchiera(i, j, i+1, j+1, i+1, j+1, i+2, j+2);// da controllare
    														m = generaStringaMossa(i, j, i+1, j+1, i+1, j+1, i+2, j+2);
    														currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    														if(currValue < bestValue){
    															bestValue = currValue;
    															mossa = m;
    														}
    														if (alfabeta > bestValue)
    															goto hell;
    														ab = bestValue;
    													}else if(scacc[(i+2)*11+j+2] == s2 && (scacchiera2.esisteCella(i+3, j+3) && (scacc[(i+3)*11+j+3] == 1 || scacc[(i+3)*11+j+3] == 0))){//NOTA: if annestati per controllare una sola volta esistenza celle per i-3, i-4 ecc ecc
    															scacFuturaClass = scacchiera2;
    															//scacFutura = scacFuturaClass.getScacchiera();
    															scacFuturaClass.aggiornaScacchiera(i, j, i+1, j+1, i+1, j+1, i+2, j+2);// da controllare, la pedina avversaria non viene toccata
    															m = generaStringaMossa(i, j, i+1, j+1, i+1, j+1, i+2, j+2);
    															currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    															if(currValue < bestValue){
    																bestValue = currValue;
    																mossa = m;
    															}
    															if (alfabeta > bestValue)
    																goto hell;
    															ab = bestValue;
    															}else if(scacc[(i+2)*11+j+2] == 1 ){//tre pedine allineate
    															if(scacchiera2.esisteCella(i+3, j+3)){
    																if(scacc[(i+3)*11+j+3] == s1){//cella vuota
    																	scacFuturaClass = scacchiera2;
    																	//scacFutura = scacFuturaClass.getScacchiera();
    																	scacFuturaClass.aggiornaScacchiera(i, j, i+2, j+2, i+1, j+1, i+3, j+3);// da controllare
    																	m = generaStringaMossa(i, j, i+2, j+2, i+1, j+1, i+3, j+3);
    																	currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																	if(currValue < bestValue){
    																		bestValue = currValue;
    																		mossa = m;
    																	}
    																	if (alfabeta > bestValue)
    																		goto hell;
    																	ab = bestValue;
    																}else if(scacc[(i+3)*11+j+3] == s2 && scacchiera2.esisteCella(i+4, j+4)){//c'e' avversario
    																		if(scacc[(i+4)*11+j+4] == 1){
    																			scacFuturaClass = scacchiera2;
    																			//scacFutura = scacFuturaClass.getScacchiera();
    																			scacFuturaClass.aggiornaScacchiera(i, j, i+2, j+2, i+1, j+1, i+3, j+3);// da controllare
    																			m = generaStringaMossa(i, j, i+2, j+2, i+1, j+1, i+3, j+3);
    																			currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																			if(currValue < bestValue){
    																				bestValue = currValue;
    																				mossa = m;
    																			}
    																			if (alfabeta > bestValue)
    																				goto hell;
    																			ab = bestValue;
    																		}else if(scacc[(i+4)*11+j+4] == s2){
    																				if(scacchiera2.esisteCella(i+5, j+5) && scacc[(i+5)*11+j+5] == 1){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i, j, i+2, j+2, i+1, j+1, i+3, j+3);// da controllare, la pedina avversaria non viene toccata
    																					m = generaStringaMossa(i, j, i+2, j+2, i+1, j+1, i+3, j+3);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}else if(scacc[(i+5)*11+j+5] == 0){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i, j, i+2, j+2, i+1, j+1, i+3, j+3);// da controllare, la pedina avversaria non viene toccata
    																					m = generaStringaMossa(i, j, i+2, j+2, i+1, j+1, i+3, j+3);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}
    																		}
    																	}else if(scacc[(i+3)*11+j+3] == s2 && scacc[(i+4)*11+j+4] == 0){
    																		scacFuturaClass = scacchiera2;
    																		//scacFutura = scacFuturaClass.getScacchiera();
    																		scacFuturaClass.aggiornaScacchiera(i, j, i+2, j+2, i+1, j+1, i+3, j+3);// da controllare
    																		m = generaStringaMossa(i, j, i+2, j+2, i+1, j+1, i+3, j+3);
    																		currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																		if(currValue < bestValue){
    																			bestValue = currValue;
    																			mossa = m;
    																		}
    																		if (alfabeta > bestValue)
    																			goto hell;
    																		ab = bestValue;
    																	}
    																}
    														}
    												}
    										}
    									}
    								}else{
    									//EST
    									if(scacchiera2.esisteCella(i, j+1)){//FIXME
    										if(scacc[i*11+j+1] == 1){//una pedina
    											scacFuturaClass = scacchiera2;
    											//scacFutura = scacFuturaClass.getScacchiera();
    											scacFuturaClass.aggiornaScacchiera(i, j, i, j, i, j+1, i, j+1);
    											m = generaStringaMossa(i, j, i, j, i, j+1, i, j+1);
    											currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    											if(currValue < bestValue){
    												bestValue = currValue;
    												mossa = m;
    											}
    											if (alfabeta > bestValue)
    												goto hell;
    											ab = bestValue;
    										}else if(scacc[i*11+j+1] == s1){// due pedine allineate
    												if(scacchiera2.esisteCella(i, j+2)){
    													if(scacc[i*11+j+2] == 1){// la cella controllata e' vuota quindi mi sposto li'
    														scacFuturaClass = scacchiera2;
    														//scacFutura = scacFuturaClass.getScacchiera();
    														scacFuturaClass.aggiornaScacchiera(i, j, i, j+1, i, j+1, i, j+2);// da controllare
    														m = generaStringaMossa(i, j, i, j+1, i, j+1, i, j+2);
    														currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    														if(currValue < bestValue){
    															bestValue = currValue;
    															mossa = m;
    														}
    														if (alfabeta > bestValue)
    															goto hell;
    														ab = bestValue;
    													}else if(scacc[i*11+j+2] == s2 && (scacchiera2.esisteCella(i, j+3) && (scacc[i*11+j+3] == 1 || scacc[i*11+j+3] == 0))){//NOTA: if annestati per controllare una sola volta esistenza celle per i-3, i-4 ecc ecc
    															scacFuturaClass = scacchiera2;
    															//scacFutura = scacFuturaClass.getScacchiera();
    															scacFuturaClass.aggiornaScacchiera(i, j, i, j+1, i, j+1, i, j+2);// da controllare, la pedina avversaria non viene toccata
    															m = generaStringaMossa(i, j, i, j+1, i, j+1, i, j+2);
    															currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    															if(currValue < bestValue){
    																bestValue = currValue;
    																mossa = m;
    															}
    															if (alfabeta > bestValue)
    																goto hell;
    															ab = bestValue;
    															}else if(scacc[i*11+j+2] == 1 ){//tre pedine allineate
    															if(scacchiera2.esisteCella(i, j+3)){
    																if(scacc[i*11+j-3] == s1){//cella vuota
    																	scacFuturaClass = scacchiera2;
    																	//scacFutura = scacFuturaClass.getScacchiera();
    																	scacFuturaClass.aggiornaScacchiera(i, j, i, j+2, i, j+1, i, j+3);// da controllare
    																	m = generaStringaMossa(i, j, i, j+2, i, j+1, i, j+3);
    																	currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																	if(currValue < bestValue){
    																		bestValue = currValue;
    																		mossa = m;
    																	}
    																	if (alfabeta > bestValue)
    																		goto hell;
    																	ab = bestValue;
    																}else if(scacc[i*11+j+3] == s2 && scacchiera2.esisteCella(i, j+4)){//c'e' avversario
    																		if(scacc[i*11+j+4] == 1){
    																			scacFuturaClass = scacchiera2;
    																			//scacFutura = scacFuturaClass.getScacchiera();
    																			scacFuturaClass.aggiornaScacchiera(i, j, i, j+2, i, j+1, i, j+3);// da controllare
    																			m = generaStringaMossa(i, j, i, j+2, i, j+1, i, j+3);
    																			currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																			if(currValue < bestValue){
    																				bestValue = currValue;
    																				mossa = m;
    																			}
    																			if (alfabeta > bestValue)
    																				goto hell;
    																			ab = bestValue;
    																		}else if(scacc[i*11+j+4] == s2){
    																				if(scacchiera2.esisteCella(i, j+5) && scacc[i*11+j+5] == 1){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i, j, i, j+2, i, j+1, i, j+3);// da controllare, la pedina avversaria non viene toccata
    																					m = generaStringaMossa(i, j, i, j+2, i, j+1, i, j+3);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}else if(scacc[i*11+j+5] == 0){
    																					scacFuturaClass = scacchiera2;
    																					//scacFutura = scacFuturaClass.getScacchiera();
    																					scacFuturaClass.aggiornaScacchiera(i, j, i, j+2, i, j+1, i, j+3);// da controllare, la pedina avversaria non viene toccata
    																					m = generaStringaMossa(i, j, i, j+2, i, j+1, i, j+3);
    																					currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																					if(currValue < bestValue){
    																						bestValue = currValue;
    																						mossa = m;
    																					}
    																					if (alfabeta > bestValue)
    																						goto hell;
    																					ab = bestValue;
    																				}
    																		}
    																	}else if(scacc[i*11+j+3] == s2 && scacc[i*11+j+4] == 0){
    																		scacFuturaClass = scacchiera2;
    																		//scacFutura = scacFuturaClass.getScacchiera();
    																		scacFuturaClass.aggiornaScacchiera(i, j, i, j+2, i, j+1, i, j+3);// da controllare
    																		m = generaStringaMossa(i, j, i, j+2, i, j+1, i, j+3);
    																		currValue = valutaMossa(scacFuturaClass, side2, depth-1, ab == -(std::numeric_limits<double>::infinity()) ? ab: -ab);
    																		if(currValue < bestValue){
    																			bestValue = currValue;
    																			mossa = m;
    																		}
    																		if (alfabeta > bestValue)
    																			goto hell;
    																		ab = bestValue;

    																}
    														}
    												}
    											}
    										}
    									}
    								}
    							}
    						}
    					}
    			}
    			hell:
    			mossaFinale = mossa;
    			return -bestValue;
    		}
    	}
    
public:
    
    AI(){
    }

    Scacchiera* getScacchiera() {
        return &scacchiera;
    }
    
    /* void distanza(){
     int dist;
     for(int i=0; i<11; i++){
     for(int j=0; j<11; j++){
     for(int k=0; k<11; k++){
     for(int l=0; l<11;l++){
     int key1=(int)(i*1000+j*100+k*10+l);
     if(distance.find(key1)==distance.end()){
     dist=(int) calcolaDistanza(i,j,k,l);
     distance.insert(key1, dist);
     }
     }
     }
     }
     }
     }*/
    
    void distanza(){
        for(int i=0; i<11; i++)
            for(int j=0; j<11; j++)
                for(int k=0; k<11; k++)
                    for(int l=0; l<11;l++){
                        dist=(i*1000+j*100+k*10+l);
                        distance[dist]=calcolaDistanza(i,j,k,l);
                    }
    }
    
    string generaStringaMossa(int i, int j, int k, int l, int m, int n, int o, int p){
        string sb;
        sb+=corrispondenzaR(i);
        sb+=to_string(j);
        sb+=corrispondenzaR(k);
        sb+=to_string(l);
        sb+=corrispondenzaR(m);
        sb+=to_string(n);
        sb+=(corrispondenzaR(o));
        sb+=to_string(p);
        //cout << "Mossa di generaStringaMossa(): "+ sb << endl;
        return sb;
    }
    
    string generaProssimaMossa(Scacchiera& s, string side, int d){
        valutaMossa(s, side, d, -(numeric_limits<double>::infinity()));
        return mossaFinale;
    }
    
    void convertiStringaMossa(string mossa){
        char vm[8];
        strncpy(vm, mossa.c_str(), sizeof(vm));
        int i = corrispondenza(mossa.at(0));//indice posizione di partenza della prima pedina del gruppo
        int k = corrispondenza(mossa.at(2));//indice posizione di partenza dell'ultima pedina del gruppo
        int j = corrispondenza(mossa.at(4));//indice posizione di arrivo della prima pedina del gruppo
        int l = corrispondenza(mossa.at(6));//indice posizione di arrivo dell'ultima pedina del gruppo
        //System.out.println("STRINGA " + i + ""+ vm[1] + "" + k + "" + vm[3] + "" + j + "" + vm[5] + "" + l + "" + vm[7]);
        int m = vm[1]-'0';
        int n = vm[3]-'0';
        int o = vm[5]-'0';
        int p = vm[7]-'0';
        //cout << "Mossa di convertiStringaMossa() "+mossa << endl;
        scacchiera.aggiornaScacchiera(i,m,k,n,j,o,l,p);
    }
};

//int main(){
//
//    Scacchiera s;
//    string mossa;
//    string x = " ABCDEFGHI ";
//    AI ai;
//    ai.distanza();
//    short *ptr = ai.getScacchiera()->getScacchiera();
//    cout << "   1 2 3 4 5 6 7 8 9  " << endl;
//    for(int i=0; i<11; i++){
//        cout << x.at(i);
//        for(int j=0; j<11;j++){
//            std::cout << ptr[i*11+j];
//            std::cout << " ";
//        }
//        std::cout << "" << std::endl;
//    }
//    cout << ptr <<endl;
//
//    mossa = ai.generaProssimaMossa(*ai.getScacchiera(), "Black", 3);
//    cout << "Your move: "+mossa << endl;
//    while(true){
//    cout << "Inserisci mossa ";
//    cin >> mossa;
//    ai.convertiStringaMossa(mossa);
//
//        
//    cout << "   1 2 3 4 5 6 7 8 9  " << endl;
//    for(int i=0; i<11; i++){
//        cout << x.at(i);
//        for(int j=0; j<11;j++){
//            std::cout << ptr[i*11+j];
//            std::cout << " ";
//        }
//        std::cout << "" << std::endl;
//    }
//        cout << ptr <<endl;
//        cout << "Pedine nere mangiate " + to_string(ai.getScacchiera()->getNereCatturate()) << endl;
//        cout << "Pedine bianche mangiate " + to_string(ai.getScacchiera()->getBiancheCatturate()) << endl;
//    }
//    return 0;
//}