#include <string>
#include <vector>
#include <iostream>
#include "RTree.h"
#include <ctime>
#include <SFML/Graphics.hpp>

using namespace std;

void print(const int& command, vector<vector<vector<pair<int, int>>>>& objects_n, string& output) {
	output.resize(0);
	output = command;

	switch (command)
	{
	case 0:// OBJECTS
		output += "|" + to_string(objects_n[0].size());
		for (auto& polygon : objects_n[0]) {
			output += "|" + to_string(polygon.size());

			for (auto& point : polygon) {
				output += "|" + to_string(point.first) + "|" + to_string(point.second);
			}
		}
		break;

	case 1: //MBRS:
		cout << " number of (objects) :" << to_string(objects_n.size()) << endl;
		output += "|" + to_string(objects_n.size());
		for (auto& objects : objects_n) {
			output += "|" + to_string(objects.size());
			cout << " number of polygons :" << to_string(objects.size()) << endl;
			for (auto& polygon : objects) {
				output += "|" + to_string(polygon.size());
				cout << " number of points :" << to_string(polygon.size()) << endl;
				for (auto& point : polygon) {
					output += "|" + to_string(point.first) + "|" + to_string(point.second);
					cout << "   point:" << to_string(point.first) << " | " << to_string(point.second) << endl;

				}
			}
			cout << endl << "--------------------" << endl;
		}
		break;
	default:
		output += "|0";
		break;
	}

	output += "|END";
	//cout<<output;
}

void print_pair(vector<pair<int, int>> output) {
	for (auto& x : output)
	{
		cout << " ( " << x.first << " , " << x.second << " ) ";
	}
}

int resol = 5;
sf::RectangleShape RecDraw(const Rect& R, int windowHeight = 500, sf::Color c = sf::Color::Blue, float grosor=1){
  int xtam = R.m_max[0] - R.m_min[0];
  int ytam = R.m_max[1] - R.m_min[1];

  sf::RectangleShape r({static_cast<float> (resol * xtam), static_cast<float>(resol * ytam)});
  r.setPosition(sf::Vector2f(static_cast<float> (resol * R.m_min[0]), static_cast<float>(windowHeight - resol * R.m_max[1])));
  sf::Color transparentColor = sf::Color(0,0,0,0);
  r.setFillColor(transparentColor);
  sf::Color outlineColor = c;
  r.setOutlineColor(outlineColor);
  r.setOutlineThickness(grosor);
  return r;
}

sf::RenderWindow window(sf::VideoMode(800, 600), "R-Tree");

void graphic(Node* node){
  if(!node) return;
  for(auto& branch : node->m_branch){
    Rect rect = branch.m_rect;
    sf::Color c = sf::Color::Blue;
    int tam = 2;
    if(node->IsLeaf() == true){
      c = sf::Color::Green;
      tam = 1;
    }
    sf::RectangleShape rectShape = RecDraw(rect,500,c,tam);
    window.draw(rectShape);

    if(branch.m_child != nullptr){
      graphic(branch.m_child);
    }
    else{
      for(const auto& point : branch.m_data){
        sf::CircleShape pointShape(3);
        pointShape.setFillColor(sf::Color::Black);
        pointShape.setPosition({static_cast<float>(resol * point.first -3), static_cast<float>(500 - resol * point.second -3 )});
        window.draw(pointShape);
      }
    }
  }
}


bool overlap(Rect* rectA, Rect* rectB){
  bool overlapX = (rectA->m_min[0] <= rectB->m_max[0] && rectA->m_max[0] >= rectA->m_min[0]);

  bool overlapY = (rectA->m_min[1] <= rectB->m_max[1] && rectA->m_max[1] >= rectA->m_min[1]);

  return overlapX && overlapY;
}


void search(Node* a_node, Rect& R, vector<Rect>& result){
  if(a_node->IsLeaf()){
    for(int i=0; i<a_node->m_count;i++){
      if(overlap(&R, &(a_node->m_branch[i].m_rect))){
        result.push_back(a_node->m_branch[i].m_rect);
      }
    }
  }
  else{
    for(int i=0; i<a_node->m_count; i++){
      if(overlap(&R, &(a_node->m_branch[i].m_rect))){
        search(a_node->m_branch[i].m_child, R, result);
      }
    }
  }
}


void PointDraw(vector<pair<int, int>> V) {
	for (int i = 0; i < V.size(); i++) {
		sf::CircleShape pointShape(3);
		pointShape.setFillColor(sf::Color::Black);
		pointShape.setPosition({ static_cast<float>(resol * V[i].first - 3), static_cast<float>(500 - resol * V[i].second - 3) });
		window.draw(pointShape);
	}
}



int main(int argc, char* argv[])
{
	//srand(static_cast<unsigned>(time(nullptr)));
	vector<vector<pair<int, int>>> vpoints;

	//   First Test
  	
	vector<pair<int, int>> points = { {20, 59}, {20, 43}, {50, 58}, {48, 67}, {105, 68}, {74, 64}, {83, 40}, {104, 54},{20,59},{20,43},{48,67},{105,68}};
	for (unsigned int i = 0; i < points.size(); i += 2) {
		vector<pair<int, int>> gr(points.begin() + i, points.begin() + i + 2);
		vpoints.push_back(gr);
	}

	//vector<pair<int, int>> points2 = { {12, 28}, {19, 15}, {40, 29}, {69, 25}, {70, 28}, {60, 15} };
	//for (unsigned int i = 0; i < points2.size(); i += 3) {
	//	vector<pair<int, int>> gr(points2.begin() + i, points2.begin() + i + 3);
	//	vpoints.push_back(gr);
	//}
	
	
	
	// 15 entradas
	/*
	vector<pair<int, int>> points = { {20, 59}, {20, 43}, {50, 58}, {48, 67}, {105, 68}, {74, 64}, {83, 40}, {104, 54}, 
                                  {12, 28}, {19, 15}, {40, 29}, {69, 25}, {70, 28}, {60, 15}, {30, 45} };

	for (unsigned int i = 0; i < points.size(); i += 3) {
    	vector<pair<int, int>> gr(points.begin() + i, points.begin() + i + 3);
    	vpoints.push_back(gr);

	}
	*/

	
	// 30 entradas
	/*
	vector<pair<int, int>> points = { {20, 59}, {20, 43}, {50, 58}, {48, 67}, {105, 68}, {74, 64}, {83, 40}, {104, 54}, 
                                  {12, 28}, {19, 15}, {40, 29}, {69, 25}, {70, 28}, {60, 15}, {30, 45}, {56, 23},
                                  {78, 90}, {12, 33}, {44, 29}, {85, 66}, {31, 48}, {22, 76}, {54, 44}, {37, 19},
                                  {66, 31}, {91, 62}, {38, 23}, {49, 78}, {61, 21}, {84, 27} };

	for (unsigned int i = 0; i < points.size(); i += 2) {
    	vector<pair<int, int>> gr(points.begin() + i, points.begin() + i + 2);
    	vpoints.push_back(gr);
	}
	*/
	/*
	for (int i = 0; i < 101; ++i) {
		vector<pair<int, int>> pointPair(2); 
		pointPair[0] = make_pair(10+rand() % 90, 10+rand() % 90);  
		pointPair[1] = make_pair(10+rand() % 90, 10+rand() % 90);  
		vpoints.push_back(pointPair); 
	}
	

	for (int i = 0; i < 101; ++i) {
		vector<pair<int, int>> pointPair(3); 

		pointPair[0] = make_pair(10 + rand() % 90, 10 + rand() % 90); 
		pointPair[1] = make_pair(10 + rand() % 90, 10 + rand() % 90); 
		pointPair[2] = make_pair(10 + rand() % 90, 10 + rand() % 90);

		vpoints.push_back(pointPair);
	}
	*/
	RTree rtree;

	string output;
	vector<vector<pair<int, int>>> objects;

	vector<vector<vector<pair<int, int>>>> objects_n;

	for (auto& x : vpoints)
	{
		//cout << "inserting " << x.size() << ": ";
		//print_pair(x);
		Rect rect = rtree.MBR(x);
		rtree.Insert(rect.m_min, rect.m_max, x);
		//cout << endl;
	}

	rtree.getMBRs(objects_n);
	//print(1, objects_n, output);


	//   Second Test
	// Adding one more region to get a bigger tree
	cout << " INSERTING ONE MORE " << endl;
	vector<pair<int, int>> ad;
	pair<int, int> ad1;
	ad1.first = 54;
	ad1.second = 12;
	pair<int, int> ad2;
	ad2.first = 53;
	ad2.second = 4;
	ad.push_back(ad2);
	ad.push_back(ad1);

	//cout << "inserting " << ad.size() << ": ";
	//print_pair(ad);
	Rect rect = rtree.MBR(ad);
	//rtree.Insert(rect.m_min, rect.m_max, ad);
	//cout << endl;


	rtree.getMBRs(objects_n);
	//print(1, objects_n, output);



	cout << " REMOVING LAST INSERTED " << endl;
	//print_pair(ad);
	rtree.Remove(rect.m_min, rect.m_max, ad);
	//cout << endl;
	 

	rtree.getMBRs(objects_n);
	//print(1, objects_n, output);
	rtree;

	Rect Search(0,0,100,100);
	vector<Rect> searc;
	search(rtree.m_root , Search, searc);


	if (searc.empty()) {
    	cout << "No se encontraron rectángulos que se solapen con el rectángulo de búsqueda." << endl;
	} else {
		cout << "Se encontraron " << searc.size() << " rectángulos que se solapan:" << endl;
		for (const Rect& rect : searc) {
			cout << "Rectángulos encontrados: [" 
				<< rect.m_min[0] << ", " << rect.m_min[1] << "] - [" 
				<< rect.m_max[0] << ", " << rect.m_max[1] << "]" << endl;
		}
	}


	sf::RectangleShape background({ 700, 500 });
	window.setFramerateLimit(0);
	background.setFillColor(sf::Color::White);
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		window.clear();
		window.draw(background);

		graphic(rtree.m_root); 

		//for (auto& it : rtree.mObjs) {		//hojas
		//	window.draw(RecDraw(Rect(rtree.MBR(it))));
		//	PointDraw(it);
		//}

		//for (auto& it : searc) { // consultas
		//	window.draw(RecDraw(it,500,sf::Color::Red,2));
		//}
		//window.draw(RecDraw(Search, 500, sf::Color::Yellow,3));

		window.display();
	}

	return 0;

}