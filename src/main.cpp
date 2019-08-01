#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <pwd.h>


#define SLEEP_TIME 500000 // 0.5s



double CPU_temp_ = 0;
double cpu_usage_ = 0;
double mem_usage_info_ = 0;
std::string log_file_dir_and_name_ = "./system_status_log";
std::string log_file_name = "/system_status_log";

// ------------------------------------------------------------

void getTime(std::string* time_string, std::string* day_string);
void * DataColl(void* args);
void * ReadCPUTemp(void* args);
void * ReadCPUUsage(void* args);
void * ReadMemInfo(void* args);

// ----------------------------------------------------------------------------
int main(void)
{

  //get home dir
  struct passwd *pw = getpwuid(getuid());
  std::string user_home_dir = pw->pw_dir;

  log_file_dir_and_name_ = user_home_dir + log_file_name;

	pthread_t tids[1];
  int ret;
  ret = pthread_create( &tids[0], NULL, ReadCPUTemp, NULL);  
  ret = pthread_create( &tids[0], NULL, ReadCPUUsage, NULL);  
  ret = pthread_create( &tids[0], NULL, ReadMemInfo, NULL);  
  ret = pthread_create( &tids[0], NULL, DataColl, NULL);  
  

  pthread_exit(NULL);

  return 0;
}

// - ----------------------------------------------------------------------------

//获取文件的字节数
long int getLen(std::ifstream &fs)
{
    fs.seekg(0,std::ios::end);
    long int len =fs.tellg();
    return len;
}


void getTime(std::string* time_string, std::string* day_string)
 {
     time_t timep;
     time (&timep);
     char tmp1[64];
     char tmp2[64];
     strftime(tmp1, sizeof(tmp1), "%Y %m %d %H:%M:%S",localtime(&timep) );
     strftime(tmp2, sizeof(tmp2), "%Y_%m_%d",localtime(&timep) );
     *time_string = tmp1;
     *day_string = tmp2;
 }


 void * DataColl(void* args)
{

  bool initial = true; 

  // check log file size
  if (initial)
  {
    std::ifstream inFile(log_file_dir_and_name_);
    if (inFile.fail()){
      std::cout<< "Log file doesn't exist : " <<  log_file_dir_and_name_ << std::endl;
    }
    else
    {
      long int log_file_size = getLen(inFile);
      if (log_file_size > (long int)(100 * 1000 * 1000)) // (100MB)
      {
        inFile.close();
        std::remove(log_file_dir_and_name_.c_str());
      }
      else
      {
        inFile.close();
      }
    }
  }



  while(1)
  {
    std::string total_info;
    std::string time_string;
    std::string day_string;
    getTime(&time_string, &day_string);

    total_info = time_string + " -- CPU Temp : " + std::to_string(CPU_temp_) + " | CPU Usage : " + std::to_string(cpu_usage_) + " | MEM Usage : " + std::to_string(mem_usage_info_);

    // std::cout << total_info << std::endl;

    std::ofstream outFile(log_file_dir_and_name_,std::ofstream::app);

    if (outFile.fail())
    {
      std::cout<< "Faile to open log file : " <<  log_file_dir_and_name_ << std::endl;
      return 0;
    }

    if (initial)
    {
      outFile << "---------------" << std::endl;
      outFile << "---------------" << std::endl;
    }

    outFile << total_info << std::endl;
    outFile.close();
    initial = false;

    usleep(SLEEP_TIME); // 0.5s
  }

  return 0;
}



// ------------------------------------------------------------
void * ReadCPUTemp(void* args)
{
  for( ; ; ){
    std::string therm;
    int i=0;
    std::ifstream inFile("/sys/class/thermal/thermal_zone0/temp");
    if (inFile.fail()){
    std::cout <<"File to open CPU temp file. "<< std::endl;
    exit(1);
    }  
    while (inFile >> therm) {
      // std::cout << therm << std::endl;

      CPU_temp_ = std::stod(therm)/1000;
      // std::cout << time_string  << " -- CPU Temp : " << std::stod(therm)/1000 << std::endl;
    }
    inFile.close();
    usleep(SLEEP_TIME); // 0.5s
  }  
  return 0;
}


// ------------------------------------------------------------
void * ReadCPUUsage(void* args)
{

  double total_jiffies_1 = 0;
  double work_jiffies_1 = 0;

  double total_jiffies_2 = 0;
  double work_jiffies_2 = 0;

  bool initial = true;

  for( ; ; ){

    std::string therm;
    int i=0;
    std::ifstream inFile("/proc/stat");
    if (inFile.fail()){
    std::cout <<"File to open CPU usage file. "<< std::endl;
    exit(1);
    }  

    double value1 = 0;
    double value2 = 0;
    double value3 = 0;
    double value4 = 0;
    double value5 = 0;
    double value6 = 0;
    double value7 = 0;

    
    int loop = 0;
    while (inFile >> therm) {
      loop ++;

      if (loop == 2)
      {
        value1 = std::stod(therm);
      }
      else if (loop == 3)
      {
        value2 = std::stod(therm);
      }
      else if (loop == 4)
      {
        value3 = std::stod(therm);
      }
      else if (loop == 5)
      {
        value4 = std::stod(therm);
      } 
      else if (loop == 6)
      {
        value5 = std::stod(therm);
      } 
      else if (loop == 7)
      {
        value6 = std::stod(therm);
      }  
      else if (loop == 8)
      {
        value7 = std::stod(therm);
      }
      else if (loop > 9)
      {
        break;
      }                 
    }

    if (initial)
    {
      total_jiffies_2 = value1 + value2 + value3 + value4 + value5 + value6 + value7;
      work_jiffies_2 = value1 + value2 + value3;
    }
    else
    {
      total_jiffies_1 = total_jiffies_2;
      work_jiffies_1 = work_jiffies_2;
      total_jiffies_2 = value1 + value2 + value3 + value4 + value5 + value6 + value7;
      work_jiffies_2 = value1 + value2 + value3;
      double work_over_period = work_jiffies_2 - work_jiffies_1;
      double total_over_period = total_jiffies_2 - total_jiffies_1;
      cpu_usage_ = work_over_period / total_over_period * 100;
    }
    
    inFile.close();
    initial = false;

    usleep(SLEEP_TIME); // 0.5s
  }  
  return 0;
}



// ------------------------------------------------------------
void * ReadMemInfo(void* args)
{

  for( ; ; ){

    std::string therm;
    int i=0;
    std::ifstream inFile("/proc/meminfo");
    if (inFile.fail()){
    std::cout <<"File to open CPU usage file. "<< std::endl;
    exit(1);
    }  

    double memtotal = 0;
    double memfree = 0;
    double memavailable = 0;
    
    int loop = 0;
    while (inFile >> therm) {
      loop ++;
      // std::cout << loop << " " << therm << std::endl;

      if (loop == 2)
      {
        memtotal = std::stod(therm);
      }
      else if (loop == 5)
      {
        memfree = std::stod(therm);
      }
      else if (loop == 8)
      {
        memavailable = std::stod(therm);
      }
      else if (loop > 10)
      {
        break;
      }                 
    }

    mem_usage_info_ = (memtotal - memavailable) / memtotal * 100;

    inFile.close();
    usleep(SLEEP_TIME); // 0.5s
  }  
  return 0;
}
