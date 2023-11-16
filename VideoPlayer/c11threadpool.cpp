#include"c11threadpool.h"

std::vector<ThreadPool::THREADS> ThreadPool::m_threadsQueue;
std::vector<ThreadPool::TASK> ThreadPool::m_tasksQueue;
std::mutex ThreadPool::m_mutex;
std::condition_variable ThreadPool::m_cond;
std::unordered_map<std::thread::id, ThreadPool::TASK> ThreadPool::thread_task;
bool ThreadPool::init(int threadsNum, int tasksNum){
	if (m_initFlag != -1)return true;//˵���̳߳��Ѿ���ʼ��
	if (threadsNum <= 0 || tasksNum <= 0) {
		std::cout << "threadsNum or tasksNum is illegal" << std::endl;
		return false;
	}
	m_maxThreads = threadsNum;
	m_maxTasks = tasksNum;
	m_freeThreads = m_maxThreads;
	m_threadsQueue.resize(m_maxThreads);
	m_tasksQueue.resize(m_maxTasks);
	for (int i = 0; i < threadsNum; i++) {
		m_threadsQueue[i].isWorking = false;
		m_threadsQueue[i].isTerminate = false;
        std::thread* _thread = new  std::thread(threadEventLoop, std::make_shared<Threads>(m_threadsQueue[i]));
		if (!_thread)return false;
		m_threadsQueue[i].id = _thread->get_id();
		_thread->detach();
	}
	m_initFlag = 1;
	return true;
}

bool ThreadPool::addTask(std::function<void(std::shared_ptr<void>)> func, std::shared_ptr<void> par){
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_size >= m_maxTasks)return false;
	//˵�������л��пռ� ����������������
	m_tasksQueue[m_pushIndex].func = func;
	m_tasksQueue[m_pushIndex].ptr = par;
	m_size++;
	m_pushIndex = (m_pushIndex + 1) % m_maxTasks;
	m_cond.notify_one();
    return true;
}

void ThreadPool::releasePool(){
	//�����ն�״̬ ͨ����������֪ͨ�����߳�
	std::unique_lock<std::mutex> lock(m_mutex);
	for (int i = 0; i < m_maxThreads; i++) {
		m_threadsQueue[i].isTerminate = true;
	}
	m_cond.notify_all();
	lock.unlock();
	std::this_thread::sleep_for(std::chrono::microseconds(500));
}
//����false ����ǰ�߳�������ʹ��  true �����߳̿���
bool ThreadPool::checkThreadValid(size_t index) {
	if (index >= m_maxThreads)return false;
	std::unique_lock <std::mutex> lock(m_mutex);
	return !m_threadsQueue.at(index).isWorking;
}

bool ThreadPool::chackThreadStatus(){
	//ÿ���2��鿴�̵߳�״̬w
	std::unique_lock<std::mutex> lock(m_mutex);
	while (m_status) {
		lock.unlock();
		if (std::cin.peek() != EOF) {
			if (std::cin.peek() == 'q')break;
		}
		lock.lock();
		for (int i = 0; i < m_maxThreads; i++) {
			printf("ID: %d,WORKING: %s", m_threadsQueue.at(i).id, (m_threadsQueue.at(i).isWorking ? "TRUE" : "FALSE"));
			if (m_threadsQueue.at(i).isWorking) {
				printf("ADDRESS: %08x\r\n", &thread_task[m_threadsQueue.at(i).id].func);
			}
			else {
				printf("ADDRESS: 0X00000000\r\n");
			}
		}
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::microseconds(2000000));
		lock.lock();
	}
	return true;
}

void ThreadPool::threadEventLoop(std::shared_ptr<void> arg) {
	std::shared_ptr<THREADS> cur_thread = std::reinterpret_pointer_cast<THREADS>(arg);
	while (true) {
		std::unique_lock<std::mutex> lock(m_mutex);//��Ϊ��Ҫ������������ ��Ҫ���� ��ֹ��Դ��������ĳ���
		while (!m_size) {//������������ʱ��ת������ �ȴ����������ķ��� ����������Ϊ����������������
			if (cur_thread->isTerminate)break;
			m_cond.wait(lock);
		}
		if (cur_thread->isTerminate)break;//������Ҫ�ȼ����̵߳�״̬��ο��ԵĻ��ڽӻ�
		TASK task = m_tasksQueue.at(m_readIndex);
		//�ͷ���Դ
		m_tasksQueue[m_readIndex].func = nullptr;
		m_tasksQueue[m_readIndex].ptr.reset();
		//���������ı�
		m_readIndex = (m_readIndex + 1) % m_maxTasks;
		m_size--;
		m_freeThreads--;
		thread_task[cur_thread->id] = task;
		lock.unlock();
		cur_thread->isWorking = true;
		task.func(task.ptr);//ִ��������
		cur_thread->isWorking = false;
		lock.lock();
		m_freeThreads++;
	}
}
