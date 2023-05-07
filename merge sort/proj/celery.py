from celery import Celery
from kombu import Exchange, Queue

#for rabbitmq
# app = Celery('proj',
#              broker='amqp://',
#              backend='rpc://',
#              include=['proj.tasks'])

#for redis
app = Celery('proj',
             broker='redis://localhost:6379/0',
             backend='redis://localhost:6379/0',
             include=['proj.tasks'])

# make custom queue for tasks with durable=False and delivery_mode=transient

app.conf.task_default_queue = 'tasks'
app.conf.task_default_exchange = 'tasks'
app.conf.task_default_routing_key = 'tasks'
app.conf.task_default_delivery_mode = 'transient'
app.conf.task_default_durable = False

app.conf.update(
    result_expires=3600,
)
app.conf.task_acks_late = True

if __name__ == '__main__':
    app.start()