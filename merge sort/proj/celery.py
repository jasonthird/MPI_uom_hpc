from celery import Celery
from kombu import Exchange, Queue
from celery.worker.control import inspect_command

#for rabbitmq
# app = Celery('proj',
#              broker='amqp://',
#              backend='rpc://',
#              include=['proj.tasks'])

#for redis
app = Celery('proj',
             broker='redis://localhost:6379/0',
             backend='redis://localhost:6379/0',
             default_exchange='tasks',
             include=['proj.tasks'])


# make custom queue for tasks with durable=False and delivery_mode=transient

app.conf.task_default_queue = 'tasks'
app.conf.task_default_exchange = 'tasks'
app.conf.task_default_routing_key = 'tasks'
app.conf.task_default_delivery_mode = 'transient'
app.conf.task_default_durable = False

# Optional configuration, see the application user guide.
app.conf.update(
    result_expires=3600,
)


if __name__ == '__main__':
    app.start()