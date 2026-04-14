/*
 *	DKU Operating System Lab (2026)
 *	    Lab1 (Scheduler Algorithm Simulator)
 *	    Student id : 32213679
 *	    Student name : 이현서
    *	Date : 2026.04.15
        Contents : FCFS, SPN, RR, Feedback scheduler 
 */

#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <algorithm>
#include "sched.h"

class FCFS : public Scheduler
{
private:
   
public:
    FCFS(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "FCFS";
    }

    int run() override
    {
    
        // 
        if (current_job_.name ==0) {

            if (job_queue_.empty()) {
                return -1; // 대기 작업 없으면 종료
            }
                // job_queue에서 current_job_에 할당
                else {
                    current_job_ = job_queue_.front();
                    job_queue_.pop();
                }
            
            // 첫 작업이 아니라면 context switch 시간 추가
            if (!end_jobs_.empty()) {
                current_time_ += switch_time_;
            }

            // first run time 기록
            current_job_.first_run_time = current_time_;
        }

        // 1s 실행
        current_job_.remain_time--;
        current_time_++;

        // 완료된 작업 end_jobs_에 push back
        if (current_job_.remain_time == 0) {
            current_job_.completion_time = current_time_;
            end_jobs_.push_back(current_job_);

            int finished_job_name = current_job_.name;
            current_job_ = Job(); // 초기화
            return finished_job_name; // 완료 작업 이름 반환

        }

        return current_job_.name; // 실행 중인 작업 이름 반환
    }
};

class SPN : public Scheduler
{
private:
    // 도착했으나 실행되지 않은 작업 저장 위한 ready_queue
    std::vector<Job> ready_queue_; 

public:
    SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "SPN";
    }
     
    int run() override
    {
        // 최소 작업 시간 순으로 job_quueue에 할당
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
            ready_queue_.push_back(job_queue_.front());
            job_queue_.pop();
         }
        
        if (current_job_.name == 0) {
            if (job_queue_.empty() && ready_queue_.empty()) {
                return -1; // 실행, 도착 예정, 대기 작업 없으면 종료
            }  
        
            // 최소 작업 시간 구하는 함수
            int min_index = 0;
            for (size_t i=1; i<ready_queue_.size(); i++) {
            if (ready_queue_[i].service_time < ready_queue_[min_index].service_time) {
                min_index = i;
                }
            }

            // 최소 작업 시간 순으로 ready_queue에서 current_job_에 할당
            current_job_ = ready_queue_[min_index];
            ready_queue_.erase(ready_queue_.begin() + min_index);
                         
            
            // 첫 작업이 아니라면 context switch 시간 추가
            if (!end_jobs_.empty()) {
                current_time_ += switch_time_;
                }

            // first run time 기록
            current_job_.first_run_time = current_time_;
         }

        // 1s 실행
        current_job_.remain_time--;
        current_time_++;

        // 완료된 작업 end_jobs_에 push back
        if (current_job_.remain_time == 0) {
            current_job_.completion_time = current_time_;
            end_jobs_.push_back(current_job_);

            int finished_job_name = current_job_.name;
            current_job_ = Job(); // 초기화
            return finished_job_name; // 완료 작업 이름 반환

        }

        return current_job_.name; // 실행 중인 작업 이름 반환

    }
};

class RR : public Scheduler
{
private:
    int time_slice_;
    int left_slice_;
    std::queue<Job> waiting_queue;
    int last_job_name_ = 0; // 직전에 실행한 작업

public:
    RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead)
    {
        name = "RR_" + std::to_string(time_slice);
        /*
         * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
         * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
         */
        time_slice_ = time_slice;
        left_slice_ = time_slice;
    }
    
    // RR 스케줄링 함수
    // 매 호출마다 도착 작업 wating_queue로 옮긴 후
    // 현재 작업이 없으면 FCFS 방식으로 다음 작업 선택
    // 1초 실행 후 작업 완료 / time slice 종료 여부 체크
    // 모든 작업 완료 시 return -1
    int run() override
    {
       // 도착 작업 옮기기
       while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
        waiting_queue.push(job_queue_.front());
        job_queue_.pop();
       }

       if (current_job_.name == 0) {
            if (waiting_queue.empty() && job_queue_.empty()) {
                return -1; // 실행, 도착 예정, 대기 작업 없으면 종료
              }
            else {
                current_job_ = waiting_queue.front();
                waiting_queue.pop();

                // 직전에 실행한 작업과 현재 작업이 다르면 context switch 시간 추가
                if (last_job_name_ != 0 && last_job_name_ != current_job_.name) {
                    current_time_ += switch_time_;
                }

                // 첫 실행에만 first run time 기록
                if (current_job_.remain_time == current_job_.service_time) { 
                    current_job_.first_run_time = current_time_;
                }

                // time slice 초기화
                left_slice_ = time_slice_; 
            }
        }

        // 1s 실행
        current_job_.remain_time--;
        current_time_++;
        left_slice_--; // 남은 time slice 감소
        last_job_name_ = current_job_.name; // 직전에 실행한 작업 이름 저장

        // 완료된 작업 end_jobs_에 push back
        if (current_job_.remain_time == 0) { 
            current_job_.completion_time = current_time_;
            end_jobs_.push_back(current_job_);

            int finished_job_name = current_job_.name;
            current_job_ = Job(); // 초기화
            return finished_job_name; // 완료 작업 이름 반환

        }

        // time slice 종료 체크
        if (left_slice_ == 0) { 
            waiting_queue.push(current_job_); // 현재 작업 waiting_queue로 이동
            int finished_job_name = current_job_.name; // time slice 종료된 작업 이름 저장
            current_job_ = Job(); // 초기화
            return finished_job_name; // time slice 종료된 작업 이름 반환
        }
    
        return current_job_.name; // 실행 중인 작업 이름 반환

    }
};


// FeedBack 스케줄러 (queue 개수 : 4 / boosting 없음)
class FeedBack : public Scheduler
{
private:
    std::vector<std::queue<Job>> queues_; // ready queue 4개
    std::vector<int> time_slices_; // 각 queue의 time slice
    int current_queue_; // 현재 실행 중인 queue
    int left_slice_; // 현재 queue의 남은 time slice
    int last_job_name_ = 0; // 직전에 실행한 작업

public:
    FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
        if (is_2i) {
            name = "FeedBack_2i";
        }
        else {
            name = "FeedBack_1";
        }
        // queue 4개 생성
        queues_.resize(4);
        
        // 각 queue의 time slice 설정
        time_slices_.resize(4);
        if (is_2i) {
            // q0: 1, q1: 2, q2: 4, q3: 8
            for (int i=0; i<4; i++) {
                time_slices_[i] = 1 << i; 
            }
        }
        else {
            // 모든 queue time slice 1
            for (int i=0; i<4; i++) {
                time_slices_[i] = 1; 
            }
        }

        // 실행 queue 없음
        current_queue_ = -1;
        left_slice_ = 0;
        last_job_name_ = 0;
    }
    // MLFQ 스케줄링 함수
    // 도착 작업을 q0에 할당
    // 가장 높은 우선 순위의 채워진 queue에서 작업 선택
    // time slice 소진 시 queue 우선 순위 낮춤
    int run() override {
        
        // 도착 작업 q0에 할당
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
            queues_[0].push(job_queue_.front());
            job_queue_.pop();
        }

        // 다음 작업 선택
        if (current_job_.name == 0) {
            bool all_empty = job_queue_.empty();
            for (int i = 0; i < 4 && all_empty; i++) {
                if (!queues_[i].empty()) 
                all_empty = false;
                }
            if (all_empty) {
                return -1; // 모든 queue, job_queue_ 비었으면 종료
                }
         
            // 가장 높은 우선 순위의 채워진 queue 찾기
            for (int i=0; i<4; i++) {
                if (!queues_[i].empty()) {
                    current_job_ = queues_[i].front();
                    queues_[i].pop();
                    current_queue_ = i;
                    break;
                    }
            } 

            // context swtich 시간 추가
            if (last_job_name_ != 0) {
                current_time_ += switch_time_;
            }
        
            // first run time 기록
            if (current_job_.remain_time == current_job_.service_time) { 
                current_job_.first_run_time = current_time_;
            }

            // time slice 설정
            left_slice_ = time_slices_[current_queue_];
         }

        // 1s 실행
         current_job_.remain_time--;
         current_time_++;
         left_slice_--; // 남은 time slice 감소
         last_job_name_ = current_job_.name; // 직전에 실행한 작업 이름 저장

        // 완료된 작업 end_jobs_에 push back
        if (current_job_.remain_time == 0) { 
            current_job_.completion_time = current_time_;
            end_jobs_.push_back(current_job_);

            int finished_job_name = current_job_.name; // 종료된 작업 이름 저장
            current_job_ = Job(); // 초기화
            return finished_job_name; // 완료 작업 이름 반환
         }
        
        // time slice 소진 시 queue 우선 순위 낮춤
        if (left_slice_ == 0) {
            int new_queue = current_queue_ + 1; // 우선 순위 조정
            if (new_queue > 3) new_queue = 3; // 가장 낮은 우선 순위는 그대로
            queues_[new_queue].push(current_job_); // 현재 작업 새로운 queue로 이동
            int finished_job_name = current_job_.name; // time slice 종료된 작업 이름 저장
            current_job_ = Job(); // 초기화
            return finished_job_name; // time slice 종료된 작업 이름 반환
        }

        return current_job_.name; // 실행 중인 작업 이름 반환
    }
};
