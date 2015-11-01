#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <pthread.h>

#define EPSILON 0.0004
#define DELTA 0.03

using namespace std;
using namespace std::chrono;

void *calculate_dsv(void *);
double *temp_holder;

// Struct used to store information about the box
struct box {
    int id;
    int x;
    int y;
    int height;
    int width;
    vector<int> top;
    vector<int> bottom;
    vector<int> left;
    vector<int> right;
    double dsv;
};

// Struct used to pass arguments to pthreads
struct arg_struct {
    int start;
    int end;
};

unordered_map<int, box> map;

// This function returns the contact length of any two boxes given the X coordinate
// of both the boxes and Lengths if the box is on top or bottom, or Width if the
// box is on left or right.
int findContact(int boxStart, int boxLength, int box2Start, int box2Length) {
    if(box2Start < boxStart) {
        return box2Start + box2Length - boxStart;
    }
    else if(box2Start == boxStart) {
        if(boxLength < box2Length) {
            return boxLength;
        }
        else {
            return box2Length;
        }
    }
    else {
        return boxStart + boxLength - box2Start;
    }
}

int main() {
    ifstream file;
    file.open("testgrid_1");
    if (!file) {
        cerr << "File not found!" << endl;
        exit(1);
    }

    int num,row,col;

    file >> num >> row >> col;
    cout << "Boxes: " << num << " Rows: " << row << " Columns: " << col << endl;

    int id;

    file >> id;

    // Reading from file till we find -1 which indicates end of the file
    while(id!=-1) {
        box Box;
        Box.id = id;

        file >> Box.y;
        file >> Box.x;
        file >> Box.height;
        file >> Box.width;

        int n;
        int temp;

        // Top neighbours
        file >> n;

        while (n > 0) {
            file >> temp;
            Box.top.push_back(temp);
            n--;
        }

        // Bottom neighbours
        file >> n;

        while (n > 0) {
            file >> temp;
            Box.bottom.push_back(temp);
            n--;
        }

        // Left neighbours
        file >> n;

        while (n > 0) {
            file >> temp;
            Box.left.push_back(temp);
            n--;
        }

        // Right neighbours
        file >> n;

        while (n > 0) {
            file >> temp;
            Box.right.push_back(temp);
            n--;
        }

        //Updating DSV
        double dsv;

        file >> dsv;

        Box.dsv = dsv;

        map.insert(make_pair(id, Box));

        file >> id;
    }

    file.close();

    int num_threads;

    // Get number of threads as a user input
    cout << "Enter the number of threads you need: ";
    cin >> num_threads;

    pthread_t threads[num_threads];

    bool unstable = true;

    int loops = 0;

    // Record the time before execution of the logic
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    struct arg_struct args[num_threads];

    // Initialize the arguments to be passed to all threads
    for (int i = 0; i < num_threads; i++) {
        args[i].start = (i*(num/num_threads));
        args[i].end = ((i+1)*(num/num_threads));
    }

    //Dissipation logic
    while(unstable) {
        loops++;
        temp_holder = new double[num];

        // Create pthreads and call the thread safe calculate_dsv function with the appropriate argument struct
        for(int i = 0; i<num_threads; i++) {
            pthread_create(&threads[i], NULL, calculate_dsv, (void *)&args[i]);
        }

        // Wait for all threads to complete
        for(int i=0; i<num_threads; i++) {
            pthread_join(threads[i], NULL);
        }

        double max = temp_holder[0];
        double min = temp_holder[0];

        // Communicate the updated temperature
        for(int j = 0; j<num; j++) {
            auto seek = map.find(j);
            seek->second.dsv = temp_holder[j];
            if(temp_holder[j] > max) {
                max = temp_holder[j];
            }
            if(temp_holder[j] < min) {
                min = temp_holder[j];
            }
        }

        // If the temperature is within the expected limits end the loop
        if((max-min) <= (DELTA * max)) {
            cout <<"Last Min: "<< min << endl;
            cout <<"Last Max: "<< max << endl;
            high_resolution_clock::time_point t2 = high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
            cout <<"Total duration: "<< duration <<" microseconds"<< endl;
            unstable = false;
        }

        delete [] temp_holder;
    }

    cout <<"Iterations: "<< loops << endl;

    return 0;
}

// Thread safe function which calculates the temparature of a box based upon the temparature of the surrounding boxes
// Arguments are the staring and ending ID of the box
void *calculate_dsv(void *arguments) {

    struct arg_struct *args = (struct arg_struct *) arguments;

    int start = args->start;
    int end = args->end;

    for(int i = start; i < end; i++) {
        auto search = map.find(i);
        box current;
        current = search->second;

        double surrounding_temp = 0.0;
        int contact_distance = 0;

        //Top
        if(current.top.size() > 0) {
            vector<int>::iterator v = current.top.begin();
            while( v != current.top.end()) {
                auto temp = map.find(*v);
                box top = temp->second;
                int contact = findContact(current.x, current.width, top.x, top.width);
                surrounding_temp += (contact * top.dsv);
                contact_distance += contact;
                v++;
            }
        }
        else {
            surrounding_temp += current.dsv * current.width;
            contact_distance += current.width;
        }

        //Bottom
        if(current.bottom.size() > 0) {
            vector<int>::iterator v = current.bottom.begin();
            while( v != current.bottom.end()) {
                auto temp = map.find(*v);
                box bottom = temp->second;
                int contact = findContact(current.x, current.width, bottom.x, bottom.width);
                surrounding_temp += (contact * bottom.dsv);
                contact_distance += contact;
                v++;
            }
        }
        else {
            surrounding_temp += current.dsv * current.width;
            contact_distance += current.width;
        }

        //Left
        if(current.left.size() > 0) {
            vector<int>::iterator v = current.left.begin();
            while( v != current.left.end()) {
                auto temp = map.find(*v);
                box left = temp->second;
                int contact = findContact(current.y, current.height, left.y, left.height);
                surrounding_temp += (contact * left.dsv);
                contact_distance += contact;
                v++;
            }
        }
        else {
            surrounding_temp += current.dsv * current.height;
            contact_distance += current.height;
        }

        //Right
        if(current.right.size() > 0) {
            vector<int>::iterator v = current.right.begin();
            while( v != current.right.end()) {
                auto temp = map.find(*v);
                box right = temp->second;
                int contact = findContact(current.y, current.height, right.y, right.height);
                surrounding_temp += (contact * right.dsv);
                contact_distance += contact;
                v++;
            }
        }
        else {
            surrounding_temp += current.dsv * current.height;
            contact_distance += current.height;
        }

        double weighted_temp = surrounding_temp / contact_distance;

        double adjusted_temp;

        // Adjust the temperature accordingly
        if(weighted_temp > current.dsv) {
            adjusted_temp = current.dsv + (EPSILON * (weighted_temp - current.dsv));
        }
        else {
            adjusted_temp = current.dsv - (EPSILON * (current.dsv - weighted_temp));
        }

        temp_holder[i] = adjusted_temp;
    }
}