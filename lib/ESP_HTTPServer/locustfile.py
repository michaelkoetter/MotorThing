from locust import HttpLocust, TaskSet, task

class Simple(TaskSet):
    @task(1)
    def index(self):
        self.client.get("/")

class ESP_HTTPServer(HttpLocust):
    host = "http://192.168.4.1"
    task_set = Simple
    min_wait=5000
    max_wait=5000
