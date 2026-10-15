import { Router } from 'express';
import { v4 as uuidv4 } from 'uuid';
import { getDb } from '../db/init';

export const fileRouter = Router();

// Upload file metadata
fileRouter.post('/upload', async (req, res) => {
  try {
    const { name, size, mime_type, conversation_id } = req.body;
    const db = await getDb();
    const id = uuidv4();
    const filePath = `/files/${id}/${name}`;

    await db.run(
      'INSERT INTO files (id, name, size, mime_type, path, conversation_id) VALUES (?, ?, ?, ?, ?, ?)',
      [id, name, size, mime_type, filePath, conversation_id]
    );

    res.json({
      id,
      name,
      size,
      mime_type,
      path: filePath,
      uploaded_at: new Date().toISOString()
    });
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});

// List conversation files
fileRouter.get('/conversation/:conversation_id', async (req, res) => {
  try {
    const db = await getDb();
    const files = await db.all(
      'SELECT * FROM files WHERE conversation_id = ? ORDER BY uploaded_at DESC',
      [req.params.conversation_id]
    );
    res.json({ files });
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});

// Delete file
fileRouter.delete('/:id', async (req, res) => {
  try {
    const db = await getDb();
    await db.run('DELETE FROM files WHERE id = ?', [req.params.id]);
    res.json({ success: true });
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});
