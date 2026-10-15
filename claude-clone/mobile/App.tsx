import React, { useEffect } from 'react';
import { View, StatusBar } from 'react-native';
import { NavigationContainer } from '@react-navigation/native';
import { createBottomTabNavigator } from '@react-navigation/bottom-tabs';
import { GestureHandlerRootView } from 'react-native-gesture-handler';
import ChatScreen from './screens/ChatScreen';
import ConversationsScreen from './screens/ConversationsScreen';
import SettingsScreen from './screens/SettingsScreen';
import { APIClient } from './services/api';

const Tab = createBottomTabNavigator();

export default function App() {
  useEffect(() => {
    // Initialize API client
    APIClient.init();
  }, []);

  return (
    <GestureHandlerRootView style={{ flex: 1 }}>
      <StatusBar barStyle="dark-content" backgroundColor="#ffffff" />
      <NavigationContainer>
        <Tab.Navigator
          screenOptions={{
            tabBarActiveTintColor: '#4f46e5',
            tabBarInactiveTintColor: '#9ca3af',
            headerShown: true,
          }}
        >
          <Tab.Screen
            name="Chat"
            component={ChatScreen}
            options={{
              title: 'Claude Clone',
              tabBarLabel: 'Chat',
            }}
          />
          <Tab.Screen
            name="Conversations"
            component={ConversationsScreen}
            options={{
              title: 'Conversations',
              tabBarLabel: 'History',
            }}
          />
          <Tab.Screen
            name="Settings"
            component={SettingsScreen}
            options={{
              title: 'Settings',
              tabBarLabel: 'Settings',
            }}
          />
        </Tab.Navigator>
      </NavigationContainer>
    </GestureHandlerRootView>
  );
}
