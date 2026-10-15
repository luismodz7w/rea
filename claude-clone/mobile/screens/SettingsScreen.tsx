import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  TextInput,
  TouchableOpacity,
  ActivityIndicator,
} from 'react-native';
import AsyncStorage from '@react-native-async-storage/async-storage';
import { APIClient } from '../services/api';

const SettingsScreen = () => {
  const [apiUrl, setApiUrl] = useState('http://localhost:3000');
  const [model, setModel] = useState('mistral');
  const [models, setModels] = useState<string[]>([]);
  const [status, setStatus] = useState('checking');
  const [savedUrl, setSavedUrl] = useState('');

  useEffect(() => {
    loadSettings();
  }, []);

  const loadSettings = async () => {
    try {
      const savedApiUrl = await AsyncStorage.getItem('api_url');
      const savedModel = await AsyncStorage.getItem('model');

      if (savedApiUrl) {
        setApiUrl(savedApiUrl);
        setSavedUrl(savedApiUrl);
        APIClient.setBaseUrl(savedApiUrl);
      }

      if (savedModel) {
        setModel(savedModel);
      }

      await checkStatus();
      await loadModels();
    } catch (error) {
      console.error('Failed to load settings:', error);
    }
  };

  const checkStatus = async () => {
    try {
      setStatus('checking');
      const isHealthy = await APIClient.checkHealth();
      if (isHealthy) {
        const llmStatus = await APIClient.checkLLMStatus();
        setStatus(llmStatus.status || 'unavailable');
      } else {
        setStatus('unavailable');
      }
    } catch (error) {
      setStatus('unavailable');
    }
  };

  const loadModels = async () => {
    try {
      const availableModels = await APIClient.getAvailableModels();
      setModels(availableModels.map((m) => m.name));
    } catch (error) {
      console.error('Failed to load models:', error);
    }
  };

  const saveSettings = async () => {
    try {
      await AsyncStorage.setItem('api_url', apiUrl);
      await AsyncStorage.setItem('model', model);
      setSavedUrl(apiUrl);
      APIClient.setBaseUrl(apiUrl);
      await checkStatus();
    } catch (error) {
      console.error('Failed to save settings:', error);
    }
  };

  const getStatusColor = () => {
    switch (status) {
      case 'healthy':
        return '#10b981';
      case 'unavailable':
        return '#ef4444';
      default:
        return '#f59e0b';
    }
  };

  const getStatusText = () => {
    switch (status) {
      case 'healthy':
        return 'Connected';
      case 'unavailable':
        return 'Disconnected';
      default:
        return 'Checking...';
    }
  };

  return (
    <ScrollView style={styles.container}>
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Backend Connection</Text>

        <View style={styles.settingGroup}>
          <Text style={styles.label}>API URL</Text>
          <TextInput
            style={styles.input}
            placeholder="http://localhost:3000"
            value={apiUrl}
            onChangeText={setApiUrl}
          />
          <Text style={styles.hint}>
            Change this if your backend is running on a different address
          </Text>
        </View>

        <View style={styles.statusContainer}>
          <View
            style={[styles.statusIndicator, { backgroundColor: getStatusColor() }]}
          />
          <Text style={styles.statusText}>{getStatusText()}</Text>
        </View>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Model Selection</Text>

        <View style={styles.settingGroup}>
          <Text style={styles.label}>Active Model</Text>
          <TextInput
            style={styles.input}
            value={model}
            onChangeText={setModel}
            placeholder="mistral"
          />
        </View>

        {models.length > 0 && (
          <View style={styles.modelsContainer}>
            <Text style={styles.label}>Available Models</Text>
            {models.map((m) => (
              <TouchableOpacity
                key={m}
                style={[
                  styles.modelItem,
                  model === m && styles.modelItemSelected,
                ]}
                onPress={() => setModel(m)}
              >
                <Text
                  style={[
                    styles.modelName,
                    model === m && styles.modelNameSelected,
                  ]}
                >
                  {m}
                </Text>
              </TouchableOpacity>
            ))}
          </View>
        )}
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>About</Text>
        <View style={styles.infoBox}>
          <Text style={styles.infoTitle}>Claude Clone</Text>
          <Text style={styles.infoText}>Version 1.0.0</Text>
          <Text style={styles.infoText}>Powered by Ollama</Text>
          <Text style={styles.infoText}>Open source • Free • Uncensored</Text>
        </View>
      </View>

      <TouchableOpacity style={styles.saveButton} onPress={saveSettings}>
        <Text style={styles.saveButtonText}>Save Settings</Text>
      </TouchableOpacity>
    </ScrollView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#ffffff',
    padding: 16,
  },
  section: {
    marginBottom: 24,
  },
  sectionTitle: {
    fontSize: 16,
    fontWeight: '700',
    color: '#1f2937',
    marginBottom: 12,
  },
  settingGroup: {
    marginBottom: 16,
  },
  label: {
    fontSize: 14,
    fontWeight: '600',
    color: '#374151',
    marginBottom: 8,
  },
  input: {
    borderWidth: 1,
    borderColor: '#d1d5db',
    borderRadius: 8,
    padding: 12,
    fontSize: 14,
    color: '#1f2937',
    backgroundColor: '#f9fafb',
  },
  hint: {
    fontSize: 12,
    color: '#9ca3af',
    marginTop: 6,
  },
  statusContainer: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingVertical: 12,
  },
  statusIndicator: {
    width: 12,
    height: 12,
    borderRadius: 6,
    marginRight: 8,
  },
  statusText: {
    fontSize: 14,
    fontWeight: '600',
    color: '#374151',
  },
  modelsContainer: {
    marginTop: 12,
  },
  modelItem: {
    paddingVertical: 10,
    paddingHorizontal: 12,
    marginBottom: 8,
    backgroundColor: '#f3f4f6',
    borderRadius: 8,
    borderWidth: 2,
    borderColor: 'transparent',
  },
  modelItemSelected: {
    backgroundColor: '#e0e7ff',
    borderColor: '#4f46e5',
  },
  modelName: {
    fontSize: 14,
    color: '#374151',
  },
  modelNameSelected: {
    color: '#4f46e5',
    fontWeight: '600',
  },
  infoBox: {
    backgroundColor: '#f3f4f6',
    padding: 16,
    borderRadius: 8,
    borderWidth: 1,
    borderColor: '#e5e7eb',
  },
  infoTitle: {
    fontSize: 16,
    fontWeight: '700',
    color: '#1f2937',
    marginBottom: 8,
  },
  infoText: {
    fontSize: 13,
    color: '#6b7280',
    marginBottom: 4,
  },
  saveButton: {
    backgroundColor: '#4f46e5',
    paddingVertical: 14,
    borderRadius: 8,
    alignItems: 'center',
    marginBottom: 32,
  },
  saveButtonText: {
    color: '#ffffff',
    fontWeight: '700',
    fontSize: 16,
  },
});

export default SettingsScreen;
