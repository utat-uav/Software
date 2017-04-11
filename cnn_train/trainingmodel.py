import os
import cv2
import math
import numpy as np
import thread
import time
import tensorflow as tf
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

variableIdx = 0

def visualizeConvWeights(matrix):
  numImages = matrix.shape[3]
  imgDim = matrix.shape[0]
  numRowsAndColumns = int(np.sqrt(float(numImages)) + 0.5)
  
  fig = plt.figure()
  fig.patch.set_facecolor('white')
  gs = gridspec.GridSpec(numRowsAndColumns, numRowsAndColumns,\
                          hspace=0.1, wspace=0.1)
  for index, g in enumerate(gs):
    if index >= numImages:
      break
    image = np.reshape(matrix[:,:,:,index], [imgDim, imgDim])
    ax = plt.subplot(g)
    ax.matshow(image, cmap = plt.cm.coolwarm)
    ax.set_xticks([])
    ax.set_yticks([])
  
  plt.show()

def weight_variable(shape, convolution):
  global variableIdx
  variableIdx = variableIdx + 1
  if convolution == True:
    return tf.get_variable("W" + str(variableIdx), shape=shape,\
              initializer=tf.contrib.layers.xavier_initializer_conv2d(\
              uniform = False))
  else:
    return tf.get_variable("W" + str(variableIdx), shape=shape,\
              initializer=tf.contrib.layers.xavier_initializer(\
              uniform = False))

def bias_variable(shape):
  initial = tf.constant(0.15, shape=shape)
  return tf.Variable(initial)
  
def conv2d(x, W):
  return tf.nn.conv2d(x, W, strides = [1, 1, 1, 1], padding = 'SAME',\
          use_cudnn_on_gpu = True)

def max_pool_2x2(x):
  return tf.nn.max_pool(x, ksize=[1, 2, 2, 1],
                        strides=[1, 2, 2, 1], padding='SAME')

# Loading data
data = np.load("Data.npy")
target = np.load("Target.npy")

numclasses = int(np.max(target) + 1)
print("there are " + str(numclasses) + " classes")

totalDataPoints = data.shape[0]
print(str(totalDataPoints) + " data points")

# Shuffling data
random = np.arange(totalDataPoints)
np.random.shuffle(random)

data = data[random]
target = target[random]

# Partitioning data
testsize = 2000
validsize = 2000
trainsize = int(round(totalDataPoints - testsize - validsize))

trainfiles = data[0:trainsize, :]
trainresult = target[0:trainsize, :]
testfiles = data[trainsize:trainsize+testsize, :]
testresult = target[trainsize:trainsize+testsize, :]
validfiles = data[trainsize+testsize:trainsize+testsize+validsize, :]
validresult = target[trainsize+testsize:trainsize+testsize+validsize, :]

print(str(trainfiles.shape[0]) + " training points")
print(str(testfiles.shape[0]) + " test points")
print(str(validfiles.shape[0]) + " validation points")

# Center calculation
center = np.mean(trainfiles)
print("data center is " + str(center))

sess = tf.InteractiveSession()

# CNN Parameters
conv_out1 = 128
conv_out2 = 192
conv_out3 = 192
fully_connected_neurons = 1152
reg_rate = 0.0005

# 1600, number of pixels of an image (40*40)
x = tf.placeholder(tf.float32, shape=[None, 1600], name = "x_in")

# numclasses, number of outputs, number of classes
y__ = tf.placeholder(tf.float32, shape=[None, 1])
y_ = tf.reshape(tf.one_hot(tf.cast(y__, tf.int32), numclasses, on_value = 1.0,\
                off_value = 0.0, axis = 1), [-1, numclasses])

# reshape for feeding into the CNN
x_image = tf.reshape(x, [-1,40,40,1]) 

# first convolutional layer   
W_conv1 = weight_variable([7, 7, 1, conv_out1], True) 
b_conv1 = bias_variable([conv_out1]) #the stack numbers have to align

# reduce the size of image by half, now become 10*10, this is only an operation
h_conv1 = tf.nn.relu(conv2d(x_image, W_conv1) + b_conv1)
h_pool1 = max_pool_2x2(h_conv1) # only an operation, not a matrix

# second convolutional layer
# the third number of stack align with previous layer and now double the stack number
W_conv2 = weight_variable([5, 5, conv_out1, conv_out2], True) 
b_conv2 = bias_variable([conv_out2])

h_conv2 = tf.nn.relu(conv2d(h_pool1, W_conv2) + b_conv2)
h_pool2 = max_pool_2x2(h_conv2)

# third convolutional layer
W_conv3 = weight_variable([3, 3, conv_out2, conv_out3], True) 
b_conv3 = bias_variable([conv_out3])

h_conv3 = tf.nn.relu(conv2d(h_pool2, W_conv3) + b_conv3)
h_pool3 = max_pool_2x2(h_conv3)

# fully connected layer
W_fc1 = weight_variable([5 * 5 * conv_out3, fully_connected_neurons], False)
b_fc1 = bias_variable([fully_connected_neurons])

h_pool3_flat = tf.reshape(h_pool3, [-1, 5 * 5 * conv_out3])
h_fc1 = tf.nn.relu(tf.matmul(h_pool3_flat, W_fc1) + b_fc1)
keep_prob = tf.placeholder(tf.float32)
h_fc1_drop = tf.nn.dropout(h_fc1, keep_prob)

# second fully connected layer
W_fc2 = weight_variable([fully_connected_neurons, fully_connected_neurons], False)
b_fc2 = bias_variable([fully_connected_neurons])

h_fc2 = tf.nn.relu(tf.matmul(h_fc1_drop, W_fc2) + b_fc2)
h_fc2_drop = tf.nn.dropout(h_fc2, keep_prob)

# readout, (1024) is number of stacks and (numclasses) is number of outputs
W_fc3 = weight_variable([fully_connected_neurons, numclasses], False)
b_fc3 = bias_variable([numclasses])

y_conv = tf.add(tf.matmul(h_fc2_drop, W_fc3), b_fc3, name = "y_out")

# Weight decay
wd_1 = tf.reduce_sum(tf.square(W_conv1))
wd_2 = tf.reduce_sum(tf.square(W_conv2))
wd_3 = tf.reduce_sum(tf.square(W_conv3))
wd_4 = tf.reduce_sum(tf.square(W_fc1))
wd_5 = tf.reduce_sum(tf.square(W_fc2))
wd_6 = tf.reduce_sum(tf.square(W_fc3))
# bd_1 = tf.reduce_sum(tf.square(b_conv1))
# bd_2 = tf.reduce_sum(tf.square(b_conv2))
# bd_3 = tf.reduce_sum(tf.square(b_conv3))
# bd_4 = tf.reduce_sum(tf.square(b_fc1))
# bd_5 = tf.reduce_sum(tf.square(b_fc2))
# bd_6 = tf.reduce_sum(tf.square(b_fc3))
weight_decay = wd_1 + wd_2 + wd_3 + wd_4 + wd_5 + wd_6# +\
                # bd_1 + bd_2 + bd_3 + bd_4 + bd_5 + bd_6

# Cross entropy
cross_entropy = tf.reduce_mean(
    tf.nn.softmax_cross_entropy_with_logits(labels=y_, logits=y_conv))

# final training and evaluating
loss = cross_entropy + reg_rate * weight_decay
train_step = tf.train.AdamOptimizer(1e-4).minimize(loss)
correct_prediction = tf.equal(tf.argmax(y_conv,1), tf.argmax(y_,1))
accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))
sess.run(tf.global_variables_initializer())

size = 360 #batch size
batch_num = int(trainsize/size)

for i in range(20):
  for j in range(batch_num):
    train = trainfiles[j*size:j*size+size]
    result = trainresult[j*size:j*size+size]    
    train_step.run(feed_dict={x: train, y__: result, keep_prob: 0.5})

    if (j%100 == 0):
      loss = sess.run(cross_entropy, feed_dict={x: validfiles, y__: validresult,\
                                                keep_prob: 1.0})
      acc = sess.run(accuracy, feed_dict={x: validfiles, y__: validresult,\
                                          keep_prob: 1.0})
      print("Validation acc = " + str(acc) + " loss = " + str(loss) +\
            " at epoch " + str(i) + " batch " + str(j))
      # print(result.shape)
      
      # wd1 = sess.run(wd_1, feed_dict={x: train, y__: result, keep_prob: 1.0})
      # wd2 = sess.run(wd_2, feed_dict={x: train, y__: result, keep_prob: 1.0})
      # wd3 = sess.run(wd_3, feed_dict={x: train, y__: result, keep_prob: 1.0})
      # wd4 = sess.run(wd_4, feed_dict={x: train, y__: result, keep_prob: 1.0})
      # wd5 = sess.run(wd_5, feed_dict={x: train, y__: result, keep_prob: 1.0})
      # wd6 = sess.run(wd_6, feed_dict={x: train, y__: result, keep_prob: 1.0})
      # 
      # print("wd1 = " + str(wd1))
      # print("wd2 = " + str(wd2))
      # print("wd3 = " + str(wd3))
      # print("wd4 = " + str(wd4))
      # print("wd5 = " + str(wd5))
      # print("wd6 = " + str(wd6))
      
  shuffle = np.arange(trainsize)
  np.random.shuffle(shuffle)

  trainfiles = trainfiles[shuffle]
  trainresult = trainresult[shuffle]

tf.train.write_graph(sess.graph_def, './', 'mlp.pb', as_text=False)

testAcc = sess.run(accuracy, feed_dict={\
                    x: testfiles, y__: testresult, keep_prob: 1.0})
print("test accuracy %g"%testAcc)


    
#next steps: save all the tensors to a tensorflow file (??)
#store the numbers in binary in the future


